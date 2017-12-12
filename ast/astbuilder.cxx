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
        if (F_ASSIGNMENT_OPERATOR()) {
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

    StructOrUnionSpecifierPtr Miyuki::AST::ASTBuilder::structOrUnionSpecifier() {
        /*struct-or-union-specifier:
            struct-or-union identifier(opt) { struct-declaration-list }
            struct-or-union identifier*/
        // NOTE: CHECK BEFORE CALL THIS FUNCTION
        TokenPtr keyword = look;
        TokenPtr id = nullptr;
        next();
        if (look->is(Tag::Identifier)) {
            id = look; next();
        }
        if (look->isNot('{')) {
            return make_shared<StructOrUnionSpecifier>(keyword, id, nullptr);
        }
        next(); //eat '{'
        StructDeclarationListPtr structDeclList = structDeclarationList();
        match('}');
        return make_shared<StructOrUnionSpecifier>(keyword, id, structDeclList);
    }

    EnumSpecifierPtr Miyuki::AST::ASTBuilder::enumSpecifier() {
        /*
        enum-specifier:
            enum identifier(opt) { enumerator-list }
            enum identifier(opt) { enumerator-list , }
            enum identifier*/
        // NOTE: CHECK BEFORE CALL THIS FUNCTION
        TokenPtr id = nullptr;
        next();
        if (look->is(Tag::Identifier)) {
            id = look; next();
        }
        if (look->isNot('{')) {
            return make_shared<EnumSpecifier>(id, nullptr);
        }
        next(); //eat '{'
        EnumeratorListPtr enumList = enumeratorList();
        match('}');
        return make_shared<EnumSpecifier>(id, enumList);
    }
}

#undef match