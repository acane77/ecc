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
                if ( look->isNot(':') ) {
                    diagError("':' token expected.", look);
                    SKIP_TO_SEMI_AND_END_THIS_SYMBOL
                }
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
        if ( FIRST_EXPRESSION() ) {
            return make_shared<CastExpression>( unaryExpression() );
        }
        CastExpressionPtr castExp = nullptr;
        if ( look->is('(') ) {
            while ( look->is('(') ) {
                // TODO: complete this after implement typeName
                assert( false && "unimplemented" );
                next();
                TypeNamePtr typeNam = nullptr;
                if ( look->isNot(')') ) {
                    diagError("')' expected.", look);
                    SKIP_TO_SEMI_AND_END_THIS_SYMBOL
                }
                castExp = make_shared<CastExpression>(typeNam, castExp);
            }
        }
        else  // PRIBLEM: it seems catch any error here.
            diagError("unexpected '{0}' token."_format(look->toSourceLiteral()), look);
        return castExp;
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
                goto SizeOfType;
            }
            // sizeof unary-expression
            return make_shared<Unary>(op, unaryExpression());
        }
        else if ( look->is(Tag::Alignof) ) {
            next();
            if ( look->is('(') ) {
                SizeOfType:
                next();
                // TODO: complete this after implement typeName
                assert( false && "unimplemented" );
                TypeNamePtr typeNam = nullptr;
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
            // TODO: uncomment here after implement
            //if ( ! FIRST_TYPE_NAME() )
            { // this is a primary-expression instead
                retract();
                goto this_is_a_primary_expression;
            }
            // TODO: complete this after implement typeName and initializerList
            assert( false && "unimplemented" );
            TypeNamePtr typeNam = nullptr;
            match(')'); match('{');
            InitializerListPtr initList = nullptr;
            match('}');
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
                match('}');
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
        if ( look->is(Tag::Identifier) || look->is(Tag::Constant) || look->is(Tag::StringLiteral) ) {
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
            TokenPtr op = look;
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
    DeclarationPtr Miyuki::AST::ASTBuilder::declaration() {
        /*declaration:
            declaration-specifiers init-declarator-listopt ;
            static_assert-declaration (not implemented)*/
        if (FIRST_DECLARATION()) {
            DeclarationSpecifierPtr spec = declarationSpecifiers();
            InitDeclaratorListPtr initDeclList = nullptr;
            if (look->isNot(';'))
                initDeclList = initDeclaratorList();
            match(';');
            return make_shared<Declaration>(spec, initDeclList);
            //return nullptr;
        }
        else diagError("invalid declaration.", look);
    }

    DeclarationSpecifierPtr Miyuki::AST::ASTBuilder::declarationSpecifiers() {
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
        }
        while (FIRST_DECLARATION_SPECIFIERS());
        return declSpec;
    }

    InitDeclaratorListPtr Miyuki::AST::ASTBuilder::initDeclaratorList() {
        /*
        init-declarator-list:
            init-declarator
            init-declarator-list , init-declarator*/
        InitDeclaratorListPtr lst = make_shared<InitDeclaratorList>();
        do {
            if (FIRST_INIT_DECLARATOR()) {
                lst->push_back(initDeclarator());
            }
            else REPORT_ERROR("invalid declaration-specifier.")
        } while (look->is(',') && next());
        return lst;
    }

    InitDeclaratorPtr Miyuki::AST::ASTBuilder::initDeclarator() {
        /*
        init-declarator:
            declarator
            declarator = initializer*/
        // already judegd
        DeclaratorPtr decr = declarator();
        InitializerPtr init = nullptr;
        if (look->is('=')) {
            next();
            init = initializer();
        }
        return make_shared<InitDeclarator>(decr, init);
    }

    InitializerPtr Miyuki::AST::ASTBuilder::initializer() {
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
            match('}');
            return make_shared<Initializer>(initList);
        }
        REPORT_ERROR("initalizer should be an expression or an initializer-list");
    }

    InitializerListPtr Miyuki::AST::ASTBuilder::initializerList() {
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
                retract();
            }
            else break;
        } while (true);
        return initList;
    }

    DesignationPtr Miyuki::AST::ASTBuilder::designation() {
        /*designation:
            designator-list = */
        if (FIRST_DESIGNATOR_LIST()) {
            DesignatorListPtr desList = designatorList();
            match('=');
            return make_shared<Designation>(desList);
        }
        REPORT_ERROR("invalid designation");
    }

    DesignatorListPtr Miyuki::AST::ASTBuilder::designatorList() {
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

    DesignatorPtr Miyuki::AST::ASTBuilder::designator() {
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

    TypeSpecifierPtr Miyuki::AST::ASTBuilder::typeSpecifier() {
        /*
        type-specifier:
            types
            atomic-type-specifier
            struct-or-union-specifier
            enum-specifier
            typedef-name -> id*/
        if (FIRST_STRUCT_OR_UNION_SPECIFIER()) {
            return structOrUnionSpecifier();
        }
        if (FIRST_ENUM_SPECIFIER()) {
            return enumSpecifier();
        }
        TypeSpecifierPtr typeSpec = make_shared<TypeSpecifier>(look);
        next();
        return typeSpec;
    }

    // NOTE: CHECK BEFORE CALL THIS FUNCTION
    StructOrUnionSpecifierPtr Miyuki::AST::ASTBuilder::structOrUnionSpecifier() {
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
        match('}');
        return make_shared<StructOrUnionSpecifier>(keyword, id, structDeclList);
    }

    // NOTE: CHECK BEFORE CALL THIS FUNCTION
    EnumSpecifierPtr Miyuki::AST::ASTBuilder::enumSpecifier() {
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
        match('}');
        return make_shared<EnumSpecifier>(id, enumList);
    }

    StructDeclarationListPtr Miyuki::AST::ASTBuilder::structDeclarationList() {
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
    StructDeclarationPtr Miyuki::AST::ASTBuilder::structDeclaration() {
        /*
        struct-declaration:
            specifier-qualifier-list struct-declarator-list(opt) ;
            static_assert-declaration (not implemented)
        */
        if (FIRST_SPECIFIER_QUALIFIER_LIST()) {
            SpecifierAndQualifierListPtr spec = specifierQualifierList();
            StructDeclaratorListPtr decl = structDeclaratorList();
            return make_shared<StructDeclaration>(spec, decl);
        }
        REPORT_ERROR("need a specifier or qualifier here.");
    }

    SpecifierAndQualifierListPtr Miyuki::AST::ASTBuilder::specifierQualifierList() {
        /*specifier-qualifier-list:
            type-specifier specifier-qualifier-list(opt)
            type-qualifier specifier-qualifier-list(opt)*/
        SpecifierAndQualifierListPtr lst = make_shared<SpecifierAndQualifierList>();
        do {
            if (FIRST_TYPE_SPECIFIER()) {
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

    StructDeclaratorListPtr Miyuki::AST::ASTBuilder::structDeclaratorList() {
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
    StructDeclaratorPtr Miyuki::AST::ASTBuilder::structDeclarator() {
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

    EnumeratorListPtr Miyuki::AST::ASTBuilder::enumeratorList() {
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

    EnumeratorPtr Miyuki::AST::ASTBuilder::enumerator() {
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
    DeclaratorPtr Miyuki::AST::ASTBuilder::declarator() {
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

    DirectDeclaratorPtr Miyuki::AST::ASTBuilder::directDeclarator(bool leftBracketHandled) {
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
        DirectDeclaratorPtr directDecl = nullptr;
        if (look->is(Tag::Identifier) && !leftBracketHandled) {
            WordTokenPtr id = static_pointer_cast<WordToken>(look); next();
            directDecl = make_shared<DirectDeclarator>(id);
        }
        else if (look->is('(')) {
            next();
            DeclaratorPtr decl = declarator();
            match(')');
            directDecl = make_shared<DirectDeclarator>(decl);
        }
        else REPORT_ERROR("expect varible or function name")

        while ( look->is('[') || look->is('(') ) {
            if (look->is('(')) {
                next();
                if (FIRST_IDENTIFIER_LIST()) {
                    // TODO: UNCOMMENT HERE AFTER IMPLEMENTED
                    // if identifier is typedef name
                    //if ( getEnvironment()->isTypedefName(look) )
                    //    goto new_style_paramster_list;
                    WordTokenListPtr lst = identifierList();
                    match(')');
                    directDecl = make_shared<DirectDeclarator>(directDecl, lst);
                }
                else if (FIRST_PARAMETER_TYPE_LIST()) {
new_style_paramster_list:
                    ParameterTypeListPtr param = parameterTypeList();
                    match(')');
                    directDecl = make_shared<DirectDeclarator>(directDecl, param);
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
                AssignmentExpressionPtr assignExpr = assignmentExpression();
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

    PointerDeclPtr Miyuki::AST::ASTBuilder::pointerDecl() {
        /*
        pointer:
            * type-qualifier-list(opt)
            * type-qualifier-list(opt) pointer
        */
        assert(look->is('*') && "for a pointer, token must be '*'");
        PointerDeclPtr ptrDecl = nullptr;
        do {
            next();
            ptrDecl = make_shared<PointerDecl>(typeQualifierList(), ptrDecl);
        } while (look->is('*'));
        return ptrDecl;
    }

    TypeQualifierListPtr Miyuki::AST::ASTBuilder::typeQualifierList() {
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

    ParameterTypeListPtr Miyuki::AST::ASTBuilder::parameterTypeList() {
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

    ParameterListPtr Miyuki::AST::ASTBuilder::parameterList() {
        /*parameter-list:
            parameter-declaration
            parameter-list , parameter-declaration*/
        ParameterDecleartionListPtr lst = make_shared<ParameterDecleartionList>();
        do {
            if (FIRST_PARAMETER_DECLARATION()) {
                // BUG: TEST_TYPEDEF_NAME
                lst->push_back(parameterDecleartion());
            }
            else REPORT_ERROR("expected parameter declaration.")
        } while (look->is(',') && next());
        return lst;
    }

    ParameterDecleartionPtr Miyuki::AST::ASTBuilder::parameterDecleartion() {
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

    IDeclaratorPtr Miyuki::AST::ASTBuilder::iDeclarator() {
        // first read the pointer, and return type related to child nodes.
        PointerDeclPtr ptrDecl = nullptr;
        if (FIRST_POINTER()) {
            ptrDecl = pointerDecl();
        }
        IDirectDeclaratorPtr dd = iDirectDeclarator();
        if (dd->getKind() == Symbol::directDeclarator) {
            return make_shared<Declarator>(static_pointer_cast<DirectDeclarator>(dd));
        }
        else {
            return make_shared<AbstractDeclarator>(ptrDecl, static_pointer_cast<DirectAbstractDeclarator>(dd));
        }
    }

    IDirectDeclaratorPtr Miyuki::AST::ASTBuilder::iDirectDeclarator() {
        if (look->is(Tag::Identifier)) {
            // it is an identitifer, reduced by direct-declarator -> identifier
            return directAbstractDeclarator(true);
        }
        if (look->is('(')) {
            next(); // eat '(', 
            // do not know what it is, continue (reduce direct-(abstract-)declarator->(abstract-)declarator)
            // if we do so, there will not be one more iDirectDeclarator
            if (look->is(Tag::Identifier)) {
                //if (is typedef-name) { // TODO: UNCOMMENT THIS AFTER IMPLEMENTED
                //    // reduced by abstract-direct-declarator -> direct-abstract-declarator(opt) ( parameter-type-list(opt) )
                //    // read 1 more token, so retract
                //    retract(); return directAbstractDeclarator(true);
                //}
                // it is an identitifer, reduced by direct-declarator -> identifier
            }
            return iDirectDeclarator();
        }
        // other situations of direct-abstract-declarator
        return directAbstractDeclarator(true);
    }

    DirectAbstractDeclaratorPtr Miyuki::AST::ASTBuilder::directAbstractDeclarator(bool leftBracketHandled) {
        /*
        direct-abstract-declarator:
         1   ( abstract-declarator ) (handled in iDirectDeclarator())
         2   direct-abstract-declaratoropt [ type-qualifier-listopt assignment-expressionopt ]
         3   direct-abstract-declaratoropt [ static type-qualifier-listopt assignment-expression ]
         4   direct-abstract-declaratoropt [ type-qualifier-list static assignment-expression ]
         5   direct-abstract-declaratoropt [ * ]
         6   direct-abstract-declaratoropt ( parameter-type-listopt )*/
        DirectAbstractDeclaratorPtr dad = nullptr;
        if (look->is('(') && !leftBracketHandled) {
            next();
            AbstractDeclaratorPtr decl = abstractDeclarator();
            match(')');
            dad = make_shared<DirectAbstractDeclarator>(decl);
        }
        do {
            if (look->is('(')) {
                // do not match production 1 because it it handled in iDirectDeclarator()
                next();
                ParameterTypeListPtr paramList = parameterTypeList();
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
                AssignmentExpressionPtr assignExpr = assignmentExpression();
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

    WordTokenListPtr Miyuki::AST::ASTBuilder::identifierList() {
        /*
        identifier-list:
            identifier
            identifier-list , identifier*/
        assert(look->is(Tag::Identifier) && "please check token before call this function");
        WordTokenListPtr lst = make_shared<WordTokenList>();
        do {
            lst->push_back(static_pointer_cast<WordToken>(look));
            next();
        } while (look->is(Tag::Identifier) && next());
        return lst;
    }

    TypeNamePtr Miyuki::AST::ASTBuilder::typeName() {
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

    AbstractDeclaratorPtr Miyuki::AST::ASTBuilder::abstractDeclarator() {
        /*abstract-declarator:
            pointer
            pointer(opt direct-abstract-declarator*/
        PointerDeclPtr ptrDecl = nullptr;
        if (FIRST_POINTER()) {
            ptrDecl = pointerDecl();
        }
        return make_shared<AbstractDeclarator>(ptrDecl, directAbstractDeclarator());
    }

}

#undef match