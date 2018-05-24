#include "astbuilder.h"
#include "firstfollowdef.h"
#include "lex/token.h"

namespace Miyuki::AST {

    using namespace Miyuki::Lex;

#define SKIP_TO_SEMI_AND_END_THIS_SYMBOL  { skipUntil({ ';' }, SkipUntilSemi); return nullptr; }
#define match(x)  { if ( look->isNot(x) ) {\
        diagError(#x " expected. at {1} line: {0}"_format( __LINE__ , __FUNCTION__ ), look);\
        SKIP_TO_SEMI_AND_END_THIS_SYMBOL\
    } else next(); }
#define matchTag(x) { if ( look->isNot(Tag::x) ) {\
        diagError("'" #x "' expected. at {1} line: {0}"_format( __LINE__ , __FUNCTION__ ), look);\
        SKIP_TO_SEMI_AND_END_THIS_SYMBOL\
    } else next(); }
#define REPORT_ERROR(msg) { diagError(msg, look); SKIP_TO_SEMI_AND_END_THIS_SYMBOL; }
#define REPORT_ERROR_1(msg, skipTo) { diagError(msg, look); skipUntil(skipTo, SkipUntilSemi | KeepSpecifiedToken); return nullptr; }
    //////////////////////// global varibles  /////////////////////////////
    // use for solve ambiguity between function declaration and function declaration
    InitializerPtr __sInitializer = nullptr;
    DeclarationSpecifierPtr __sDeclSpec = nullptr;
    DeclaratorPtr   __sDeclaeator = nullptr;
    bool _sFlagIsFunctionDef = false;   // is function definition
    bool _sFlagAllowFunctionDef = false; // check if is called by external-declaration
    int  _sIndexOfInitDecl = 0; // index of initial decl

    // use for reconize typedef-name
    bool __sHasTypedefSpecifier = false;

    ////////////////////////    expressions   /////////////////////////////

    ConditionalExpressionPtr ASTBuilder::constantExpression() {
        // constant-expression:  conditional-expression
        return conditionalExpression();
    }

    ConditionalExpressionPtr ASTBuilder::conditionalExpression() {
        /* conditional-expression:
                logical-OR-expression
                logical-OR-expression ? expression : conditional-expression
         */
        if ( FIRST_EXPRESSION() ) {
            LogicalORExpressionPtr orExpr = logicalORExpression();
            if ( look->is('?') ) {
                next();
                ExpressionPtr expr = expression();
                match(':');
                ConditionalExpressionPtr condExpr = conditionalExpression();
                return make_shared<ConditionalExpression>( orExpr, expr, condExpr );
            }
            else return make_shared<ConditionalExpression>( orExpr );
        }
        else {
            diagError("invalid expression", look);
            SKIP_TO_SEMI_AND_END_THIS_SYMBOL
        }
    }

    CommaExpressionPtr ASTBuilder::expression() {
        /* expression:
              assignment-expression
              expression , assignment-expression
         */
        AssignmentExpressionPtr assign = assignmentExpression();
        CommaExpressionPtr exp = make_shared<CommaExpression>(nullptr, assign);
        while ( look->is(',') ) {
            next();
            exp = make_shared<CommaExpression>(exp, assignmentExpression());
        }
        return exp;
    }

    LogicalORExpressionPtr ASTBuilder::logicalORExpression() {
        // logical-OR-expression:
        //      logical-AND-expression
        //      logical-OR-expression || logical-AND-expression
        LogicalANDExpressionPtr andExpr = logicalANDExpression();
        LogicalORExpressionPtr exp = make_shared<LogicalORExpression>(andExpr, nullptr);
        while ( look->is(Tag::Or) ) {
            next();
            exp = make_shared<LogicalORExpression>(logicalANDExpression(), exp);
        }
        return exp;
    }

    LogicalANDExpressionPtr ASTBuilder::logicalANDExpression() {
        // logical-AND-expression:
        //      inclusive-OR-expression
        //      logical-AND-expression && inclusive-OR-expression
        ArithmeticPtr arith = inclusiveORExpression();
        LogicalANDExpressionPtr exp = make_shared<LogicalANDExpression>(arith, nullptr);
        while ( look->is(Tag::And) ) {
            next();
            exp = make_shared<LogicalANDExpression>(inclusiveORExpression(), exp);
        }
        return exp;
    }

    ArithmeticPtr ASTBuilder::inclusiveORExpression() {
        // inclusive-OR-expression:
        //      exclusive-OR-expression
        //      inclusive-OR-expression | exclusive-OR-expression
        ArithmeticPtr exp = exclusiveORExpression();
        while ( look->is('|') ) {
            TokenPtr op = look;
            next();
            exp = make_shared<Arithmetic>(op, exp, exclusiveORExpression());
        }
        return exp;
    }

    ArithmeticPtr ASTBuilder::exclusiveORExpression() {
        // exclusive-OR-expression:
        //      AND-expression
        //      exclusive-OR-expression ^ AND-expression
        ArithmeticPtr exp = andExpression();
        while ( look->is('^') ) {
            TokenPtr op = look;
            next();
            exp = make_shared<Arithmetic>(op, exp, andExpression());
        }
        return exp;
    }

    ArithmeticPtr ASTBuilder::andExpression() {
        // AND-expression:
        //      equality-expression
        //      AND-expression & equality-expression
        ArithmeticPtr exp = equalityExpression();
        while ( look->is('&') ) {
            TokenPtr op = look;
            next();
            exp = make_shared<Arithmetic>(op, exp, equalityExpression());
        }
        return exp;
    }

    ArithmeticPtr ASTBuilder::equalityExpression() {
        // equality-expression:
        //      relational-expression
        //      equality-expression == relational-expression
        //      equality-expression != relational-expression
        ArithmeticPtr exp = relationalExpression();
        while ( look->is(Tag::Equal) || look->is(Tag::NotEqual) ) {
            TokenPtr op = look;
            next();
            exp = make_shared<Arithmetic>(op, exp, relationalExpression());
        }
        return exp;
    }

    ArithmeticPtr ASTBuilder::relationalExpression() {
        //relational-expression:
        //      shift-expression
        //      relational-expression < shift-expression
        //      relational-expression > shift-expression
        //      relational-expression <= shift-expression
        //      relational-expression >= shift-expression
        ArithmeticPtr exp = shiftExpression();
        while ( look->is('>') || look->is('<') || look->is(Tag::GreaterThanEqual) || look->is(Tag::LessThanEqual) ) {
            TokenPtr op = look;
            next();
            exp = make_shared<Arithmetic>(op, exp, shiftExpression());
        }
        return exp;
    }

    ArithmeticPtr ASTBuilder::shiftExpression() {
        // shift-expression:
        //      additive-expression
        //      shift-expression << additive-expression
        //      shift-expression >> additive-expression
        ArithmeticPtr exp = additiveExpression();
        while ( look->is(Tag::LeftShift) || look->is(Tag::RightShift) ) {
            TokenPtr op = look;
            next();
            exp = make_shared<Arithmetic>(op, exp, additiveExpression());
        }
        return exp;
    }

    ArithmeticPtr ASTBuilder::additiveExpression() {
        // additive-expression:
        //      multiplicative-expression
        //      additive-expression + multiplicative-expression
        //      additive-expression - multiplicative-expression
        ArithmeticPtr exp = multiplicativeExpression();
        while ( look->is('+') || look->is('-') ) {
            TokenPtr op = look;
            next();
            exp = make_shared<Arithmetic>(op, exp, multiplicativeExpression());
        }
        return exp;
    }

    ArithmeticPtr ASTBuilder::multiplicativeExpression() {
        // multiplicative-expression:
        //      cast-expression
        //      multiplicative-expression * cast-expression
        //      multiplicative-expression / cast-expression
        //      multiplicative-expression % cast-expression
        CastExpressionPtr castExpr = castExpression();
        ArithmeticPtr exp = make_shared<Arithmetic>( make_shared<Token>(Tag::Cast), castExpr, nullptr );
        while ( look->is('*') || look->is('/') || look->is('%') ) {
            TokenPtr op = look;
            next();
            exp = make_shared<Arithmetic>(op, exp, castExpression());
        }
        return exp;
    }

    CastExpressionPtr ASTBuilder::castExpression() {
        // cast-expression:
        //      unary-expression
        //      ( type-name ) cast-expression
        if (look->is('(')) {
            next(); //read more token
            if (! FIRST_TYPE_NAME() ) {
                retract();
                goto this_is_an_expression;
            }

            if (look->is(Tag::Identifier) && !isTypedefName(look) ) {
                retract();
                goto this_is_an_expression;
            }
            
            TypeNamePtr typeNam = typeName();
            match(')');
            /// TODO: 把匿名数组放到这个地方，因为 (type-name) 都是相同内容
            ///       遇到 '{' 那么是匿名数组，否则是类型转换。
            return make_shared<CastExpression>(typeNam, castExpression());
        }

        else if ( FIRST_EXPRESSION() ) {
this_is_an_expression:
            return make_shared<CastExpression>( unaryExpression() );
        }
        
        else  // PRIBLEM: it seems catch any error here.
            diagError("unexpected '{0}' token."_format(look->toSourceLiteral()), look);
        assert(false && "invalid come to here");
    }

    UnaryPtr ASTBuilder::unaryExpression() {
        // unary-expression
        //      postfix-expression
        //      ++ unary-expression
        //      -- unary-expression
        //      unary-operator cast-expression
        //      sizeof unary-expression
        //      sizeof ( type-name )
        //      alignof ( type-name )
        TokenPtr op = look;
        if ( look->is(Tag::Increase) || look->is(Tag::Decrease) ) {
            next();
            // TODO: Tail recursive into circulation
            return make_shared<Unary>(op, unaryExpression());
        }
        else if ( look->is(Tag::Sizeof) ) {
            next();
            if ( look->is('(') ) {
                // look ahead 1 token
                next(); 
                if (FIRST_TYPE_NAME())
                    goto SizeOfType;
                retract();
            }
            // sizeof unary-expression
            return make_shared<Unary>(op, unaryExpression());
        }
        else if ( look->is(Tag::Alignof) ) {
            next();
            if ( look->is('(') ) {
                next();
SizeOfType:
                TypeNamePtr typeNam = typeName();
                match(')');
                return make_shared<TypeInfoExpression>(op, typeNam);
            }
            else {
                diagError("'(' expected.", look);
                SKIP_TO_SEMI_AND_END_THIS_SYMBOL
            }
        }
        else if ( FIRST_UNARY_OPERATOR() ) {
            next();
            CastExpressionPtr castExpr = castExpression();
            return make_shared<Unary>(op, castExpr);
        }
        else if ( FIRST_EXPRESSION() ) {
            return postfixExpression();
        }
        else {
            diagError("unexpected '{0}' token."_format( look->toSourceLiteral() ), look);
            SKIP_TO_SEMI_AND_END_THIS_SYMBOL;
        }
    }

    PostfixExpressionPtr ASTBuilder::postfixExpression() {
        // postfix-expression:
        //      primary-expression
        //      postfix-expression [ expression ]
        //      postfix-expression ( argument-expression-listopt )
        //      postfix-expression . identifier
        //      postfix-expression -> identifier
        //      postfix-expression ++
        //      postfix-expression --
        //      ( type-name ) { initializer-list }
        //      ( type-name ) { initializer-list , }
        PostfixExpressionPtr exp = nullptr;
        if ( look->is('(') ) {
            next();
            
            if ( ! FIRST_TYPE_NAME() )
            { // this is a primary-expression instead
                retract();
                goto this_is_a_primary_expression;
            }

            if (look->is(Tag::Identifier) && !isTypedefName(look)) {
                // this is a primary-expression instead
                // id maybe a typedef -name
                retract();
                goto this_is_a_primary_expression;
            }
            
            TypeNamePtr typeNam = typeName();
            match(')'); match('{');
            InitializerListPtr initList = initializerList();
            match(0x7d);
            exp = make_shared<AnonymousArray>(typeNam, initList);
        }
        else if ( FIRST_PRIMARY_EXPRESSION() ) {
this_is_a_primary_expression:
            exp = primaryExpression();
        }
        else {
            diagError("unexpected '{0}' token."_format( look->toSourceLiteral() ), look);
            SKIP_TO_SEMI_AND_END_THIS_SYMBOL;
        }
        while ( look->is('.') || look->is(Tag::PointerAccess) || look->is(Tag::Increase) || look->is(Tag::Decrease) || look->is('[') || look->is('(') ) {
            if ( look->is(Tag::Increase) || look->is(Tag::Decrease) ) {
                // ++ --
                TokenPtr op = look; next();
                exp = make_shared<PostfixExpression>( op, nullptr, exp );
            }
            else if ( look->is(Tag::PointerAccess) || look->is('.') ) {
                // struct access
                TokenPtr op = look; next();
                TokenPtr id = look; match(Tag::Identifier);
                exp = make_shared<StructAccess>( op, nullptr, exp, static_pointer_cast<WordToken>(id) );
            }
            else if ( look->is('(') ) {
                // function call
                next();
                ArgumentExpressionListPtr argList = nullptr;
                if ( look->isNot(')') )
                    argList = argumentExpressionList();
                match(')');
                exp = make_shared<FunctionCall>( nullptr, nullptr, exp, argList );
            }
            else if ( look->is('[') ) {
                // array access
                next();
                ExpressionPtr expr = expression();
                match(']');
                exp = make_shared<ArrayAccess>( nullptr, nullptr, exp, expr );
            }
        }
        return exp;
    }

    PrimaryExpressionPtr ASTBuilder::primaryExpression() {
        // primary-expression:
        //      identifier
        //      constant
        //      string-literal
        //      ( expression )
        //      generic-selection
        if ( look->is(Tag::Identifier) || look->is(Tag::Constant) || look->is(Tag::Character) || look->is(Tag::StringLiteral) ) {
            TokenPtr tok = look;  next();
            return make_shared<PrimaryExpression>(tok);
        }
        if ( look->is('(') ) {
            next();
            ExpressionPtr expr = expression();
            match(')');
            return make_shared<PrimaryExpression>(expr);
        }
        diagError("invalid primary-expression", look);
        SKIP_TO_SEMI_AND_END_THIS_SYMBOL
    }

    AssignmentExpressionPtr ASTBuilder::assignmentExpression() {
        // skipUntil({ ';', ',' }, SkipUntilSemi); return nullptr;
        // assignment-expression:
        //      conditional-expression
        //      unary-expression assignment-operator assignment-expression
        // I replace unary-expression with conditional-expression, and judge if it is a lvalue
        AssignmentExpressionPtr exp = make_shared<AssignmentExpression>( conditionalExpression() );
        while ( F_ASSIGNMENT_OPERATOR() ) {
            TokenPtr op = look; next();
            exp = make_shared<AssignmentExpression>( op, conditionalExpression(), exp );
        }
        return exp;
    }

    ArgumentExpressionListPtr ASTBuilder::argumentExpressionList() {
        // argument-expression-list:
        //      assignment-expression
        //      argument-expression-list , assignment-expression
        ArgumentExpressionListPtr ret = make_shared<ArgumentExpressionList>( nullptr, assignmentExpression() );
        while ( look->is(',') ) {
            next();
            ret = make_shared<ArgumentExpressionList>( ret, assignmentExpression() );
        }
        return ret;
        // skip until , or ) or ; in assi
    }

    ////////////////////////    statements   /////////////////////////////
    DeclarationPtr ASTBuilder::declaration() {
        /*declaration:
            declaration-specifiers init-declarator-listopt ;
            static_assert-declaration (not implemented)*/
        if (FIRST_DECLARATION()) {
            DeclarationSpecifierPtr spec = declarationSpecifiers();
            InitDeclaratorListPtr initDeclList = nullptr;
            if (look->isNot(';'))
                initDeclList = initDeclaratorList();
            // return if is function definition
            if (_sFlagIsFunctionDef) return nullptr;
            match(';');
            __sHasTypedefSpecifier = false;
            return make_shared<Declaration>(spec, initDeclList);
            //return nullptr;
        }
        else diagError("invalid declaration.", look);
    }

    DeclarationSpecifierPtr ASTBuilder::declarationSpecifiers() {
        /*
        declaration-specifiers:
            storage-class-specifier declaration-specifiersopt
            type-specifier declaration-specifiersopt
            type-qualifier declaration-specifiersopt
            function-specifier declaration-specifiersopt
            alignment-specifier declaration-specifiersopt (unimplemented)
        */
        DeclarationSpecifierPtr declSpec = nullptr;
        do {
            SpecifierAndQualifierPtr spec;
            if (FIRST_STORAGE_CLASS_SPECIFIER()) {
                if (look->is(Tag::Typedef))
                    __sHasTypedefSpecifier = true;
                spec = make_shared<StorageClassSpecifier>(look);  next();
            }
            else if (FIRST_TYPE_QUALIFIER()) {
                spec = make_shared<TypeQualifier>(look); next();
            }
            else if (FIRST_TYPE_SPECIFIER()) {
                spec = typeSpecifier();
            }
            else if (FIRST_FUNCTION_SPECIFIER()) {
                spec = make_shared<FunctionSpecifier>(look); next();
            }
            else REPORT_ERROR("invalid declaration-specifier.")

            declSpec = make_shared<DeclarationSpecifier>(spec, declSpec);

            if (look->is(Tag::Identifier) && !isTypedefName(look))
                break;
        }
        while (FIRST_DECLARATION_SPECIFIERS());
        __sDeclSpec = declSpec;
        return declSpec;
    }

    InitDeclaratorListPtr ASTBuilder::initDeclaratorList() {
        /*
        init-declarator-list:
            init-declarator
            init-declarator-list , init-declarator*/
        InitDeclaratorListPtr lst = make_shared<InitDeclaratorList>();
        _sIndexOfInitDecl = 0; __sInitializer = nullptr;
        do {
            if (FIRST_INIT_DECLARATOR()) {
                lst->push_back(initDeclarator());
                
                // use for resolve ambiguity between function declaration and function 
                // definition.
                // If this is the 1st declarator and next token is follow(declaration-specifiers declarator)
                // and no initializer, this is a function definition, and destory this tree
                // (6.9.1) function-definition:
                //              declaration-specifiers declarator declaration-listopt compound-statement
                // follow(declaration-specifiers declarator)
                if (FIRST_DECLARATION_LIST() || FIRST_COMPOUND_STATEMENT()) {
                    // may be function definition
                    if (!_sIndexOfInitDecl  && __sInitializer == nullptr) {
                        if (!_sFlagAllowFunctionDef) {
                            diagError("function definition is not allowed here.", look);
                            skipUntil({ '{' }, SkipUntilEndOfFile);
                            return nullptr;
                        }
                        _sFlagIsFunctionDef = true;  // set function definition flag to true and 
                                                     // this flag will be processed later
                        break;
                    }
                }
                
            }
            else REPORT_ERROR("invalid declaration-specifier.")
            _sIndexOfInitDecl++;
        } while (look->is(',') && next());
        return lst;
    }

    InitDeclaratorPtr ASTBuilder::initDeclarator() {
        /*
        init-declarator:
            declarator
            declarator = initializer*/
        // already judegd
        DeclaratorPtr decr = declarator();
        InitializerPtr init = nullptr;
        __sDeclaeator = decr;
        if (look->is('=')) {
            next();
            init = initializer();
            __sInitializer = init;
        }
        return make_shared<InitDeclarator>(decr, init);
    }

    InitializerPtr ASTBuilder::initializer() {
        /*
        initializer:
            assignment-expression
            { initializer-list }
            { initializer-list , }*/
        if (FIRST_EXPRESSION()) {
            return make_shared<Initializer>(assignmentExpression());
        }
        if (look->is('{')) {
            next(); 
            InitializerListPtr initList = initializerList();
            if (look->is(',')) next();
            match(0x7d);
            return make_shared<Initializer>(initList);
        }
        REPORT_ERROR("initalizer should be an expression or an initializer-list");
    }

    InitializerListPtr ASTBuilder::initializerList() {
        /*initializer-list:
            designation(opt) initializer
            initializer-list , designation(opt) initializer*/
        if (!FIRST_INITIALIZER_LIST()) {
            REPORT_ERROR("invalid initializer");
        }
        InitializerListPtr initList = nullptr;
        do {
            DesignationPtr des = nullptr;
            if (FIRST_DESIGNATION()) { des = designation(); }
            initList = make_shared<InitializerList>(initializer(), des, initList);
            if (look->is(',')) {
                next();  // look ahead 2 tokens, because of comma
                if (look->is('}')) {
                    retract();
                    break;
                }
            }
            else break;
        } while (true);
        return initList;
    }

    DesignationPtr ASTBuilder::designation() {
        /*designation:
            designator-list = */
        if (FIRST_DESIGNATOR_LIST()) {
            DesignatorListPtr desList = designatorList();
            match('=');
            return make_shared<Designation>(desList);
        }
        REPORT_ERROR("invalid designation");
    }

    DesignatorListPtr ASTBuilder::designatorList() {
        /*designator-list:
            designator
            designator-list designator*/
        if (!FIRST_DESIGNATOR()) {
            REPORT_ERROR("invalid designator");
        }
        DesignatorListPtr desList = make_shared<DesignatorList>();
        do {
            desList->push_back(designator());
        } while (FIRST_DESIGNATOR());
        return desList;
    }

    DesignatorPtr ASTBuilder::designator() {
        /*designator:
             [ constant-expression ]
             . identifier*/
        if (look->is('[')) {
            next();
            return make_shared<Designator>(constantExpression());
        }
        else if (look->is('.')) {
            next(); 
            WordTokenPtr id = static_pointer_cast<WordToken>( look ); next();
            return make_shared<Designator>(id);
        }
        assert(false && "expected '[' or '.'");
    }

    TypeSpecifierPtr ASTBuilder::typeSpecifier() {
        /*
        type-specifier:
            types
            atomic-type-specifier
            struct-or-union-specifier
            enum-specifier
            typedef-name        id*/
        if (FIRST_STRUCT_OR_UNION_SPECIFIER()) {
            return structOrUnionSpecifier();
        }
        if (FIRST_ENUM_SPECIFIER()) {
            return enumSpecifier();
        }
        if (look->is(Tag::Identifier) && !isTypedefName(look)) {
            REPORT_ERROR("'{0}' does not name a type"_format(look->toSourceLiteral()));
        }
        TypeSpecifierPtr typeSpec = make_shared<TypeSpecifier>(look);
        next();
        return typeSpec;
    }

    // NOTE: CHECK BEFORE CALL THIS FUNCTION
    StructOrUnionSpecifierPtr ASTBuilder::structOrUnionSpecifier() {
        /*struct-or-union-specifier:
            struct-or-union identifier(opt) { struct-declaration-list }
            struct-or-union identifier*/
        TokenPtr keyword = look;
        TokenPtr id = nullptr;
        next();
        if (look->is(Tag::Identifier)) {
            id = look; next();
        }
        if (look->isNot('{')) {
            if (id == nullptr) {
                REPORT_ERROR("{0} name expected"_format(keyword->toSourceLiteral()));
            }
            return make_shared<StructOrUnionSpecifier>(keyword, id, nullptr);
        }
        next(); //eat '{'
        StructDeclarationListPtr structDeclList = structDeclarationList();
        match(0x7d);
        return make_shared<StructOrUnionSpecifier>(keyword, id, structDeclList);
    }

    // NOTE: CHECK BEFORE CALL THIS FUNCTION
    EnumSpecifierPtr ASTBuilder::enumSpecifier() {
        /*
        enum-specifier:
            enum identifier(opt) { enumerator-list }
            enum identifier(opt) { enumerator-list , }
            enum identifier*/
        TokenPtr id = nullptr;
        next();
        if (look->is(Tag::Identifier)) {
            id = look; next();
        }
        if (look->isNot('{')) {
            if (id == nullptr) {
                REPORT_ERROR("enum name expected");
            }
            return make_shared<EnumSpecifier>(id, nullptr);
        }
        next(); //eat '{'
        EnumeratorListPtr enumList = enumeratorList();
        match(0x7d);
        return make_shared<EnumSpecifier>(id, enumList);
    }

    StructDeclarationListPtr ASTBuilder::structDeclarationList() {
        /*struct-declaration-list:
            struct-declaration
            struct-declaration-list struct-declaration*/
        if (!FIRST_STRUCT_DECLARATION_LIST()) REPORT_ERROR("struct declaration expected.")
        StructDeclarationListPtr lst = make_shared<StructDeclarationList>();
        do {
            lst->push_back(structDeclaration());
        } while (FIRST_STRUCT_DECLARATION());
        return lst;
    }

    // NOTE: CHECK BEFORE CALL THIS FUNCTION
    StructDeclarationPtr ASTBuilder::structDeclaration() {
        /*
        struct-declaration:
            specifier-qualifier-list struct-declarator-list(opt) ;
            static_assert-declaration (not implemented)
        */
        if (FIRST_SPECIFIER_QUALIFIER_LIST()) {
            SpecifierAndQualifierListPtr spec = specifierQualifierList();
            StructDeclaratorListPtr decl = nullptr;
            if (FIRST_STRUCT_DECLARATOR_LIST())
                decl = structDeclaratorList();
            match(';');
            return make_shared<StructDeclaration>(spec, decl);
        }
        REPORT_ERROR("need a specifier or qualifier here.");
    }

    SpecifierAndQualifierListPtr ASTBuilder::specifierQualifierList() {
        /*specifier-qualifier-list:
            type-specifier specifier-qualifier-list(opt)
            type-qualifier specifier-qualifier-list(opt)*/
        SpecifierAndQualifierListPtr lst = make_shared<SpecifierAndQualifierList>();
        do {
            if (FIRST_TYPE_SPECIFIER()) {
                if (look->is(Tag::Identifier) && !isTypedefName(look))
                    break;
                lst->push_back(typeSpecifier());
            }
            else if (FIRST_TYPE_QUALIFIER()) {
                lst->push_back(make_shared<TypeQualifier>(look));
                next();
            }
            else REPORT_ERROR("type specifier or type specifier expected")
        } while (FIRST_SPECIFIER_QUALIFIER_LIST());
        return lst;
    }

    StructDeclaratorListPtr ASTBuilder::structDeclaratorList() {
        /*struct-declarator-list:
            struct-declarator
            struct-declarator-list , struct-declarator*/
        StructDeclaratorListPtr lst = make_shared<StructDeclaratorList>();
        do {
            if (FIRST_STRUCT_DECLARATOR()) {
                lst->push_back(structDeclarator());
            }
            else REPORT_ERROR("invalid struct-declarator.")
        } while (look->is(',') && next());
         return lst;
    }

    // NOTE: CHECK BEFORE CALL THIS FUNCTION
    StructDeclaratorPtr ASTBuilder::structDeclarator() {
        /*struct-declarator:
            declarator
            declarator(opt) : constant-expression*/
        DeclaratorPtr decl = nullptr;
        if (FIRST_DECLARATOR()) {
            decl = declarator();
        }
        if (look->isNot(':')) {
            if (decl == nullptr) {
                REPORT_ERROR("member name expected");
            }
            return make_shared<StructDeclarator>(decl, nullptr);
        }
        next();
        return make_shared<StructDeclarator>(decl, constantExpression());
    }

    EnumeratorListPtr ASTBuilder::enumeratorList() {
        /*enumerator-list:
            enumerator
            enumerator-list , enumerator*/
        EnumeratorListPtr lst = make_shared<EnumeratorList>();
        do {
            if (FIRST_ENUMERATOR()) {
                lst->push_back(enumerator());
            }
            else REPORT_ERROR("expected identifier.")
        } while (look->is(',') && next());
        return lst;
    }

    EnumeratorPtr ASTBuilder::enumerator() {
        /*enumerator:
            enumeration-constant
            enumeration-constant = constant-expression*/
        TokenPtr enumConst = look;
        ConstantExpressionPtr constExpr = nullptr;
        matchTag(Identifier);
        if (look->is('=')) {
            next();
            constExpr = constantExpression();
        }
        return make_shared<Enumerator>(look, constExpr);
    }

    // NOTE: CHECK BEFORE CALL THIS FUNCTION
    DeclaratorPtr ASTBuilder::declarator() {
        /*
        declarator:
            pointer(opt) direct-declarator
        */
        PointerDeclPtr ptrDecl = nullptr;
        if (FIRST_POINTER()) {
            ptrDecl = pointerDecl();
        }
        return make_shared<Declarator>(ptrDecl, directDeclarator());
    }

    DirectDeclaratorPtr ASTBuilder::directDeclarator(bool leftBracketHandled, DirectDeclaratorPtr ptr) {
        /*direct-declarator:
            identifier
            ( declarator )
            direct-declarator [ type-qualifier-list(opt) assignment-expressionopt ]
            direct-declarator [ static type-qualifier-list(opt) assignment-expression ]
            direct-declarator [ type-qualifier-list static assignment-expression ]
            direct-declarator [ type-qualifier-list(opt) * ]
            direct-declarator ( parameter-type-list )
            direct-declarator ( identifier-list(opt) )
        */
        DirectDeclaratorPtr directDecl = ptr;
        if (look->is(Tag::Identifier)) {
            WordTokenPtr id = static_pointer_cast<WordToken>(look); next();
            if (__sHasTypedefSpecifier) {
                typedefNames.push_back(id->name);
            }
            directDecl = make_shared<DirectDeclarator>(id);
        }
        else if (look->is('(') && !leftBracketHandled) {
            next();
            DeclaratorPtr decl = declarator();
            match(')');
            directDecl = make_shared<DirectDeclarator>(decl);
        }
        else if (ptr);
        else REPORT_ERROR("expect varible or function name")

        while ( look->is('[') || look->is('(') ) {
            if (look->is('(')) {
                next();
                if (FIRST_IDENTIFIER_LIST()) {
                    // if identifier is typedef name
                    if ( isTypedefName(look) )
                        goto new_style_paramster_list;
                    WordTokenListPtr lst = identifierList();
                    directDecl = make_shared<DirectDeclarator>(directDecl, lst);
                }
                else if (FIRST_PARAMETER_TYPE_LIST()) {
new_style_paramster_list:
                    ParameterTypeListPtr param = parameterTypeList();
                    directDecl = make_shared<DirectDeclarator>(directDecl, param);
                }
				else if (look->is(')')) {
					directDecl = make_shared<DirectDeclarator>(directDecl, (ParameterTypeListPtr)nullptr);
				}
                match(')');
            }
            else if (look->is('[')) {
                next();
                bool firstPositionIsStatic = look->is(Tag::Static) && next();
                bool isStatic = firstPositionIsStatic;
                bool hasPoiner = false;
                int productID;
                TypeQualifierListPtr typeQualList = nullptr;
                if (FIRST_TYPE_QUALIFIER_LIST()) {
                    typeQualList = typeQualifierList();
                }
                // because FIRST({*}) and FIRST(assignment-expression) both have * in set,
                // so we must look ahead 2 tokens. 
                if (look->is('*')) {
                    next();
                    if (look->is( ']' )) {
                        if (firstPositionIsStatic) {
                            REPORT_ERROR("unexpected static");
                        }
                        directDecl = make_shared<DirectDeclarator>(directDecl, typeQualList); //4
                        continue;
                    }
                    retract();
                }
                if (look->is(Tag::Static)) {
                    if (firstPositionIsStatic) {
                        REPORT_ERROR("unexpected static");
                    }
                    isStatic = true;
                    next();
                }
                AssignmentExpressionPtr assignExpr = nullptr;
                if (FIRST_EXPRESSION()) {
                    assignExpr = assignmentExpression();
                }
                match(']');
                if (!isStatic) productID = 2;
                else if (firstPositionIsStatic) productID = 3;
                else { 
                    productID = 4;
                    if (typeQualList == nullptr) {
                        REPORT_ERROR("expect type-qualifier");
                    }
                }
                directDecl = make_shared<DirectDeclarator>(directDecl, isStatic, assignExpr, typeQualList, productID);
            }
        }
        return directDecl;
    }

    PointerDeclPtr ASTBuilder::pointerDecl() {
        /*
        pointer:
            * type-qualifier-list(opt)
            * type-qualifier-list(opt) pointer
        */
        assert(look->is('*') && "for a pointer, token must be '*'");
        PointerDeclPtr ptrDecl = nullptr;
        do {
            next();
            TypeQualifierListPtr qual = nullptr;
            if (FIRST_TYPE_QUALIFIER_LIST())
                qual = typeQualifierList();
            ptrDecl = make_shared<PointerDecl>(qual, ptrDecl);
        } while (look->is('*'));
        return ptrDecl;
    }

    TypeQualifierListPtr ASTBuilder::typeQualifierList() {
        /*
        type-qualifier-list:
            type-qualifier
            type-qualifier-list type-qualifier
        */
        assert(FIRST_TYPE_QUALIFIER_LIST() && "invalid type-qualifier");
        TypeQualifierListPtr typeQual = make_shared<TypeQualifierList>();
        do {
            typeQual->push_back(make_shared<TypeQualifier>(look));
            next();
        } while (FIRST_TYPE_QUALIFIER_LIST());
        return typeQual;
    }

    ParameterTypeListPtr ASTBuilder::parameterTypeList() {
        /*
        parameter-type-list:
            parameter-list
            parameter-list , ...
        */
        assert(FIRST_PARAMETER_TYPE_LIST() && "please check token before call this function");
        ParameterListPtr lst = parameterList();
        bool isParameterVarible = false;
        if (look->is(',')) {
            next(); match(Tag::Ellipsis);
            isParameterVarible = true;
        }
        return make_shared<ParameterTypeList>(lst, isParameterVarible);
    }

    ParameterListPtr ASTBuilder::parameterList() {
        /*parameter-list:
            parameter-declaration
            parameter-list , parameter-declaration*/
        ParameterDecleartionListPtr lst = make_shared<ParameterDecleartionList>();
        do {
            if (FIRST_PARAMETER_DECLARATION()) {
                if (look->is(Tag::Identifier) && !isTypedefName(look)) {
                    diagError("'{0}' does not name a type"_format(look->toSourceLiteral()), look);
                    skipUntil({ ',' }, SkipUntilSemi);
                    continue;
                }
                lst->push_back(parameterDecleartion());
            }
            else if (look->is(Tag::Ellipsis)) {
                // for , ... (varible parameter)
                retract();
                break;
            }
            else REPORT_ERROR("expected parameter declaration.")
        } while (look->is(',') && next());
        return lst;
    }

    ParameterDecleartionPtr ASTBuilder::parameterDecleartion() {
        /*
        parameter-declaration:
            declaration-specifiers declarator
            declaration-specifiers abstract-declarator(opt)
        */
        assert(FIRST_PARAMETER_DECLARATION() && "please check token before call this function");

        DeclarationSpecifierPtr spec = declarationSpecifiers();
        if (FOLLOW_PARAMETER_DECLARATION()) {
            // no abstract-declarator(opt)
            return make_shared<ParameterDecleartion>(spec);
        }
        IDeclaratorPtr decl = iDeclarator();
        if (!decl) return nullptr;
        if (decl->getKind() == Symbol::declarator) {
            return make_shared<ParameterDecleartion>(spec, static_pointer_cast<Declarator>(decl));
        }
        return make_shared<ParameterDecleartion>(spec, static_pointer_cast<AbstractDeclarator>(decl));
        /*
        // ABONDANDED LOOP IMPLEMENTATION
        deque<TokenPtr> S;
        PointerDeclPtr ptrDecl = nullptr;
        TypeQualifierListPtr typeQual = nullptr;

        while (FIRST_POINTER() || look->is('(')) {
            S.push_back(look); next();
        }

        if (look->is(Tag::Identifier)) {
            // is an identifier, means it is a direct-declarator
            DirectDeclaratorPtr dd = make_shared<DirectDeclarator>(look);
            DeclarationPtr d;
            next();
            while (S.size()) {
                TokenPtr x = S.back();
                S.pop_back(); // pop a (

                // test pointer
                ptrDecl = nullptr;
                x = S.back();
                
                deque<TokenPtr> T;
                if (x->is('*')) {
                    // while x is tokens in expansion of pointer
                    while (x->is('*') || x->is(Tag::Const) || x->is(Tag::Restrict) || x->is(Tag::Volatile) || x->is(Tag::KAtomic) ) {
                        T.push_back(x);
                        S.pop_back(); x = S.back();
                    }
                }

                // TODO: parse <pointer> & <type-qualifier-list>
                while (T.size()) {
                    TokenPtr y = T.back();
                    T.pop_back(); // pop a *
                    if (y->isNot('*')) REPORT_ERROR("missing pointer")
                    ptrDecl = nullptr;
                    do {
                        TypeQualifierListPtr Q = nullptr;
                        if (x->is)
                        // TODO: parse <pointer> & <type-qualifier-list>
                    } while (x->is('*'));
                }
            }
        }
        else {

        }*/
    }

    IDeclaratorPtr ASTBuilder::iDeclarator() {
        // first read the pointer, and return type related to child nodes.
        PointerDeclPtr ptrDecl = nullptr;
        if (FIRST_POINTER()) {
            ptrDecl = pointerDecl();
        }
        if (!FIRST_DIRECT_DECLARATOR() && !FIRST_DIRECT_ABSTRACT_DECLARATOR()) {
            // if next token is not a direct(-abstract)-declarator, it must
            // reduce by abstract-declarator->pointer
            return make_shared<AbstractDeclarator>(ptrDecl, nullptr);
        }
        IDirectDeclaratorPtr dd = iDirectDeclarator();
        if (!dd) return nullptr;
        if (dd->getKind() == Symbol::directDeclarator) {
            return make_shared<Declarator>(static_pointer_cast<DirectDeclarator>(dd));
        }
        else {
            return make_shared<AbstractDeclarator>(ptrDecl, static_pointer_cast<DirectAbstractDeclarator>(dd));
        }
    }

    IDirectDeclaratorPtr ASTBuilder::iDirectDeclarator() {
        if (look->is(Tag::Identifier)) {
            // it is an identitifer, reduced by direct-declarator -> identifier
            return directDeclarator(true);
        }
        if (look->is('(')) {
            next(); // eat '(', 
            // do not know what it is, continue (reduce direct-(abstract-)declarator->(abstract-)declarator)
            // if we do so, there will not be one more iDirectDeclarator
            if (look->is(Tag::Identifier)) {
                if (isTypedefName(look)) {
                    // reduced by abstract-direct-declarator -> direct-abstract-declarator(opt) ( parameter-type-list(opt) )
                    // read 1 more token, so retract
                    retract(); return directAbstractDeclarator(true);
                }
                // it is an identitifer, reduced by direct-declarator -> identifier
            }
            else if (FIRST_PARAMETER_TYPE_LIST() || look->is(')')) {
                // reduced by direct-abstract-declaratoropt ( parameter-type-listopt )
                retract();
                return directAbstractDeclarator(true);
            }
            else if (look->is('*')) {
                // reduce by -> ( (abstract-)declarator )
                IDeclaratorPtr idecl = iDeclarator();
                match(')');
                if (!idecl) return nullptr;

                if (idecl->getKind() == Symbol::declarator) {
                    DirectDeclaratorPtr dd = make_shared<DirectDeclarator>(static_pointer_cast<Declarator>(idecl));
                    if (look->is('[') || look->is('(')) {
                        // If there is a follow-up structure
                        dd = directDeclarator(true, dd);
                    }
                    return dd;
                }
                else {
                    DirectAbstractDeclaratorPtr dad = make_shared<DirectAbstractDeclarator>(static_pointer_cast<AbstractDeclarator>(idecl));
                    if (look->is('[') || look->is('(')) {
                        // If there is a follow-up structure
                        dad = directAbstractDeclarator(true, dad);
                    }
                    return dad;
                }
            }
            IDirectDeclaratorPtr r = iDirectDeclarator();
            match(')');
            return r;
        }
        // other situations of direct-abstract-declarator
        return directAbstractDeclarator(true);
    }

    DirectAbstractDeclaratorPtr ASTBuilder::directAbstractDeclarator(bool leftBracketHandled, DirectAbstractDeclaratorPtr ptr) {
        /*
        direct-abstract-declarator:
         1   ( abstract-declarator ) (handled in iDirectDeclarator())
         2   direct-abstract-declaratoropt [ type-qualifier-listopt assignment-expressionopt ]
         3   direct-abstract-declaratoropt [ static type-qualifier-listopt assignment-expression ]
         4   direct-abstract-declaratoropt [ type-qualifier-list static assignment-expression ]
         5   direct-abstract-declaratoropt [ * ]
         6   direct-abstract-declaratoropt ( parameter-type-list(opt) )*/
        DirectAbstractDeclaratorPtr dad = ptr;
        if (look->is('(') && !leftBracketHandled) {
            next();
            AbstractDeclaratorPtr decl = abstractDeclarator();
            match(')');
            dad = make_shared<DirectAbstractDeclarator>(decl);
        }
        else if (ptr);

        do {
            if (look->is('(')) {
                // do not match production 1 because it it handled in iDirectDeclarator()
                next();
                ParameterTypeListPtr paramList = nullptr;
                if (FIRST_PARAMETER_TYPE_LIST())
                    paramList = parameterTypeList();
                match(')');
                dad = make_shared<DirectAbstractDeclarator>(dad, paramList);
            }
            else if (look->is('[')) {
                next();

                if (look->is('*')) {
                    next();  match(']');
                    dad = make_shared<DirectAbstractDeclarator>(dad);
                    continue;
                }

                bool firstPositionIsStatic = look->is(Tag::Static) && next();
                bool isStatic = firstPositionIsStatic;
                int productID;
                TypeQualifierListPtr typeQualList = nullptr;

                if (FIRST_TYPE_QUALIFIER_LIST()) {
                    typeQualList = typeQualifierList();
                }

                if (look->is(Tag::Static)) {
                    if (firstPositionIsStatic) {
                        REPORT_ERROR("unexpected static");
                    }
                    isStatic = true;
                    next();
                }
                AssignmentExpressionPtr assignExpr = nullptr;
                if (FIRST_EXPRESSION()) {
                    assignExpr = assignmentExpression();
                }
                
                match(']');
                if (!isStatic) productID = 1;
                else if (firstPositionIsStatic) productID = 2;
                else {
                    productID = 3;
                    if (typeQualList == nullptr) {
                        REPORT_ERROR("expect type-qualifier");
                    }
                }
                dad = make_shared<DirectAbstractDeclarator>(dad, assignExpr, typeQualList, isStatic, productID);
            }
            else assert(false && "is not a direct-abstract-declarator");
        } while (look->is('(') || look->is('['));
        return dad;
    }

    WordTokenListPtr ASTBuilder::identifierList() {
        /*
        identifier-list:
            identifier
            identifier-list , identifier*/
        assert(look->is(Tag::Identifier) && "please check token before call this function");
        WordTokenListPtr lst = make_shared<WordTokenList>();
        do {
            if (look->isNot(Tag::Identifier))
                REPORT_ERROR_1("expect identifier.", { ')' });
            lst->push_back(static_pointer_cast<WordToken>(look));
            next();
        } while (look->is(',') && next());
        return lst;
    }

    TypeNamePtr ASTBuilder::typeName() {
        /*
        type-name:
            specifier-qualifier-list abstract-declarator(opt)
        */
        SpecifierAndQualifierListPtr spec = specifierQualifierList();
        AbstractDeclaratorPtr adecl = nullptr;
        if (FIRST_ANSTRACT_DECLARATOR()) {
            adecl = abstractDeclarator();
        }
        return make_shared<TypeName>(spec, adecl);
    }

    AbstractDeclaratorPtr ASTBuilder::abstractDeclarator() {
        /*abstract-declarator:
            pointer
            pointer(opt direct-abstract-declarator*/
        PointerDeclPtr ptrDecl = nullptr;
        DirectAbstractDeclaratorPtr dad = nullptr;
        if (FIRST_POINTER()) {
            ptrDecl = pointerDecl();
        }
        if (FIRST_DIRECT_ABSTRACT_DECLARATOR()) {
            dad = directAbstractDeclarator();
        }
        if (!(ptrDecl || dad)) {
            // at least one of ptrDecl and dad should be non-Null
            REPORT_ERROR("invalid abstract declarator");
        }
        return make_shared<AbstractDeclarator>(ptrDecl, dad);
    }

    bool ASTBuilder::isTypedefName(const TokenPtr& tok)
    {
        if (tok->isNot(Tag::Identifier)) {
            return false;
        }
        return find(typedefNames.begin(), typedefNames.end(), static_pointer_cast<WordToken>(tok)->name) != typedefNames.end();
    }

    /////////////////  statements //////////////////
    StatementPtr ASTBuilder::statement() {
        /*statement:
            labeled-statement
            compound-statement
            expression-statement
            selection-statement
            iteration-statement
            jump-statement*/
        //assert( FIRST_STATEMENT() && "check brfore this statement" );

        if (FIRST_LABELED_STATEMENT()) {
            // if look is id, it conflects with Expression-Statement
            TokenPtr prev = look;
            next();
            if (prev->is(Tag::Identifier) && look->isNot(':')) {
                // is an expression-statement
                retract();
                goto this_is_an_expression_statement;
            }
            retract();
            return labeledStatement();
        }
this_is_an_expression_statement:
        if (FIRST_EXPRSSION_STATEMENT()) {
            return expressionStatement();
        }
        else if (FIRST_COMPOUND_STATEMENT()) {
            return compoundStatement();
        }
        else if (FIRST_SELECTION_STATEMENT()) {
            return selectionStatement();
        }
        else if (FIRST_ITERATION_STATEMENT()) {
            return iterationStatement();
        }
        else if (FIRST_JUMP_STATEMENT()) {
            return jumpStatement();
        }
        else REPORT_ERROR("invalid statement");
    }

    LabeledStatementPtr ASTBuilder::labeledStatement() {
        /*labeled-statement:
                identifier : statement
                case constant-expression : statement
                default : statement*/
        assert(FIRST_LABELED_STATEMENT() && "please check token before use this function");
        if (look->is(Tag::Identifier)) {
            TokenPtr id = look;
            next();  match(':');
            StatementPtr stmt = statement();
            return make_shared<LabeledStatement>(stmt, static_pointer_cast<WordToken>(id));
        }
        else if (look->is(Tag::Case)) {
            next();
            ConstantExpressionPtr constExpr = constantExpression();
            match(':');
            StatementPtr stmt = statement();
            return make_shared<LabeledStatement>(stmt, constExpr);
        }
        else if (look->is(Tag::Default)) {
            next();  match(':');
            StatementPtr stmt = statement();
            return make_shared<LabeledStatement>(stmt);
        }
        assert(false);
    }

    CompoundStatementPtr ASTBuilder::compoundStatement() {
        /*compound-statement:
                { block-item-list(opt) }*/
        assert(FIRST_COMPOUND_STATEMENT() && "please check token before use this function");
        match('{');
        BlockItemListPtr blkItemList = nullptr;
        if (FIRST_BLOCK_ITEM_LIST()) {
            blkItemList = blockItemList();
        }
        match(0x7d);
        return make_shared<CompoundStatement>(blkItemList);
    }

    BlockItemListPtr ASTBuilder::blockItemList() {
        /*block-item-list:
            block-item
            block-item-list block-item
        */
        assert(FIRST_BLOCK_ITEM_LIST() && "check");
        BlockItemListPtr lst = make_shared<BlockItemList>();
        do {
            BlockItemPtr blkItem = blockItem();
            lst->push_back(blkItem);
        } while (FIRST_BLOCK_ITEM()); //optmize to !FOLLOW(block-item-list)
        return lst;
    }

    BlockItemPtr ASTBuilder::blockItem() {
        /*block-item:
            declaration
            statement*/
        assert(FIRST_BLOCK_ITEM() && "check");
        if (FIRST_STATEMENT()) {
            // if next token is ID, conflets. so check
            if (look->is(Tag::Identifier) && isTypedefName(look)) {
                goto this_is_a_declaration;
            }
            return make_shared<BlockItem>(statement());
        }
        else if (FIRST_DECLARATION()) {
this_is_a_declaration:
            return make_shared<BlockItem>(declaration());
        }
    }

    ExpressionStatementPtr ASTBuilder::expressionStatement() {
        /*expression-statement:
            expressionopt ;*/
        assert(FIRST_EXPRSSION_STATEMENT() && "check");
        ExpressionPtr expr = nullptr;
        if (look->isNot(';'))
            expr = expression();
        match(';');
        return make_shared<ExpressionStatement>(expr);
    }

    StatementPtr ASTBuilder::selectionStatement() {
        /*selection-statement:
                if ( expression ) statement
                if ( expression ) statement else statement
                switch ( expression ) statement
        */
        if (look->is(Tag::If)) {
            next(); match('(');
            ExpressionPtr cond = expression();
            match(')');
            StatementPtr stmt = statement();
            ElsePtr elseStmt = nullptr;
            if (look->is(Tag::Else)) {
                next();
                elseStmt = make_shared<Else>(statement());
            }
            return make_shared<If>(cond, stmt, elseStmt);
        }
        else if (look->is(Tag::Switch)) {
            next(); match('(');
            ExpressionPtr expr = expression();
            match(')');
            StatementPtr stmt = statement();
            return make_shared<Switch>(expr, stmt);
        }
        else assert(false && "require if or switch");
    }

    StatementPtr ASTBuilder::iterationStatement() {
        /*iteration-statement:
                while ( expression ) statement
                do statement while ( expression ) ;
                for ( expressionopt ; expressionopt ; expressionopt ) statement
                for ( declaration expressionopt ; expressionopt ) statement
        */
        if (look->is(Tag::While)) {
            next(); match('(');
            ExpressionPtr cond = expression();
            match(')');
            StatementPtr stmt = statement();
            return make_shared<While>(cond, stmt);
        }
        else if (look->is(Tag::Do)) {
            next();
            StatementPtr stmt = statement();
            matchTag(While); 
            match('('); ExpressionPtr cond = expression(); match(')'); match(';');
            return make_shared<DoWhile>(cond, stmt);
        }
        else if (look->is(Tag::For)) {
            next(); match('(');
            DeclarationPtr decl = nullptr;
            ExpressionPtr init = nullptr, cond = nullptr, inc = nullptr;
            if (FIRST_DECLARATION() && (look->isNot(Tag::Identifier) || isTypedefName(look))) {
                // is declaration
                decl = declaration();
            }
            else if (FIRST_EXPRESSION()) {
                // else is expression
                init = expression();
                match(';');
            } 
            else { // else init nothing
                match(';');
            }
            
            if (FIRST_EXPRESSION())
                cond = expression();
            match(';');
            if (FIRST_EXPRESSION())
                inc = expression();
            match(')');
            StatementPtr stmt = statement();
            return make_shared<For>(decl, init, cond, inc, stmt);
        }
        else assert(false && "expected while, do, for");
    }

    StatementPtr ASTBuilder::jumpStatement() {
        /*jump-statement:
                goto identifier ;
                continue ;
                break ;
                return expressionopt ;*/
        if (look->is(Tag::Goto)) {
            next(); TokenPtr id = look;
            matchTag(Identifier); match(';');
            return make_shared<Goto>(id);
        }
        else if (look->is(Tag::Continue)) {
            next(); match(';');
            return make_shared<Continue>();
        }
        else if (look->is(Tag::Break)) {
            next(); match(';');
            return make_shared<Break>();
        }
        else if (look->is(Tag::Return)) {
            next();
            ExpressionPtr expr = nullptr;
            if (FIRST_EXPRESSION()) {
                expr = expression();
            }
            return make_shared<Return>(expr);
        }
    }

    TranslationUnitPtr ASTBuilder::translationUnit() {
        /*translation-unit:
                external-declaration
                translation-unit external-declaration*/
        TranslationUnitPtr unit = nullptr;
        while (true) {
            if (FIRST_EXTERNAL_DECLARATION()) {
                unit = make_shared<TranslationUnit>(externalDeclaration(), unit);
                continue;
            }
            if (look->is(Tag::EndOfFile)) {
                break;
            }
            // report error and skip to next sentense
            diagError("only declarations and function definition are allowed at globe scope", look);
            skipUntil({ ';' }, SkipUntilSemi);
        }
        return unit;
    }

    ExternalDeclarationPtr ASTBuilder::externalDeclaration() {
        /*external-declaration:
                function-definition
                declaration*/
        assert(FIRST_EXTERNAL_DECLARATION() && "check before call");

        // because FIRST(function-definition) = FIRST(declaration)
        // so we cannot determine which branch to go only by FIRST set
        _sFlagAllowFunctionDef = true;
        DeclarationPtr decl = declaration();
        _sFlagAllowFunctionDef = false;
        if (_sFlagIsFunctionDef) {
            // if meet a function definition
            FunctionDefinitionPtr funcDef = functionDefinition();
            return make_shared<ExternalDeclaration>(funcDef);
        }
        // otherwise is a declaration
        return make_shared<ExternalDeclaration>(decl);
    }

    FunctionDefinitionPtr ASTBuilder::functionDefinition() {
        /*function-definition:
                declaration-specifiers declarator declaration-listopt compound-statement
        */
        assert(_sFlagIsFunctionDef && "this is not a functin definition");
        _sFlagIsFunctionDef = false;

        DeclarationSpecifierPtr declSpec = __sDeclSpec;
        DeclaratorPtr decr = __sDeclaeator;
        DeclarationListPtr declLst = nullptr;
        CompoundStatementPtr compStmt = nullptr;

        // continue parsing at declaration-list(opt) compound-statement
        if (FIRST_DECLARATION_LIST()) {
            declLst = declarationList();
        }
        if (look->is('{'))
            compStmt = compoundStatement();
        else REPORT_ERROR("expected '=', ',', ';' before '{0}' token."_format(look->toSourceLiteral()))
        return make_shared<FunctionDefinition>(declSpec, decr, declLst, compStmt);
    }

    DeclarationListPtr ASTBuilder::declarationList() {
        /*declaration-list:
                declaration
                declaration-list declaration*/
        DeclarationListPtr lst = make_shared<DeclarationList>();
        do {
            lst->push_back(declaration());
        } while (FIRST_DECLARATION());
        return lst;
    }
}

#undef match