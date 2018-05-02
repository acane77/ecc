#ifndef _MIYUKI_ASTBUILDER_H
#define _MIYUKI_ASTBUILDER_H

#include "parse/ifparser.h"
#include "astsym.h"

// this file defines AST Builder, which builds Anstract Syntax Tree
// this class is common use in both preprocessor and parser

namespace Miyuki::AST {

    using namespace Miyuki::Parse;

    class ASTBuilder : public IParser {
    public:
        virtual TokenPtr buildXXX() { assert( false && "do not use this function, just a sample function." ); }

        // typedef names (identifier)
        vector<string> typedefNames;
        bool isTypedefName(const TokenPtr& tok);

        // expression  (ref A.2.1)
        CommaExpressionPtr       expression();
        ConditionalExpressionPtr constantExpression();
        ConditionalExpressionPtr conditionalExpression();
        LogicalORExpressionPtr   logicalORExpression();
        LogicalANDExpressionPtr  logicalANDExpression();
        ArithmeticPtr            inclusiveORExpression();
        ArithmeticPtr            exclusiveORExpression();
        ArithmeticPtr            andExpression();
        ArithmeticPtr            equalityExpression();
        ArithmeticPtr            relationalExpression();
        ArithmeticPtr            shiftExpression();
        ArithmeticPtr            additiveExpression();
        ArithmeticPtr            multiplicativeExpression();
        CastExpressionPtr        castExpression();
        UnaryPtr                 unaryExpression();
        PostfixExpressionPtr     postfixExpression();
        PrimaryExpressionPtr     primaryExpression();
        AssignmentExpressionPtr  assignmentExpression();
        ArgumentExpressionListPtr argumentExpressionList();

        // declaration  (ref A.2.2)
        DeclarationPtr           declaration();
        DeclarationSpecifierPtr  declarationSpecifiers();
        InitDeclaratorListPtr    initDeclaratorList();
        InitDeclaratorPtr        initDeclarator();
        TypeSpecifierPtr         typeSpecifier();
        StructOrUnionSpecifierPtr structOrUnionSpecifier();
        EnumSpecifierPtr         enumSpecifier();
        StructDeclarationListPtr structDeclarationList();
        StructDeclarationPtr     structDeclaration();
        SpecifierAndQualifierListPtr specifierQualifierList();
        StructDeclaratorListPtr  structDeclaratorList();
        StructDeclaratorPtr      structDeclarator();
        EnumeratorListPtr        enumeratorList();
        EnumeratorPtr            enumerator();
        DeclaratorPtr            declarator();
        DirectDeclaratorPtr      directDeclarator(bool leftBracketHandled = false, DirectDeclaratorPtr ptr = nullptr);
        PointerDeclPtr           pointerDecl();
        TypeQualifierListPtr     typeQualifierList();
        ParameterTypeListPtr     parameterTypeList();
        ParameterListPtr         parameterList();
        ParameterDecleartionPtr  parameterDecleartion();
        WordTokenListPtr         identifierList();
        TypeNamePtr              typeName();
        AbstractDeclaratorPtr    abstractDeclarator();
        DirectAbstractDeclaratorPtr directAbstractDeclarator(bool leftBracketHandled = false, DirectAbstractDeclaratorPtr ptr = nullptr);
        InitializerPtr           initializer();
        InitializerListPtr       initializerList();
        DesignationPtr           designation();
        DesignatorListPtr        designatorList();
        DesignatorPtr            designator();
        IDeclaratorPtr           iDeclarator();
        IDirectDeclaratorPtr     iDirectDeclarator();

        // statements
        StatementPtr             statement();
        LabeledStatementPtr      labeledStatement();
        CompoundStatementPtr     compoundStatement();
        BlockItemListPtr         blockItemList();
        BlockItemPtr             blockItem();
        ExpressionStatementPtr   expressionStatement();
        StatementPtr             selectionStatement();
        StatementPtr             iterationStatement();
        StatementPtr             jumpStatement();

        // external definition
        TranslationUnitPtr       translationUnit();
        ExternalDeclarationPtr   externalDeclaration();
        FunctionDefinitionPtr    functionDefinition();
        DeclarationListPtr       declarationList();
    };

}

#endif