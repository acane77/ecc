#ifndef _MIYUKI_SYMBOLS_H
#define _MIYUKI_SYMBOLS_H

#include "ast/type.h"
#include "lex/token.h"
#include "common/ptrdef.h"

namespace Miyuki::AST {

    DEFINE_SHARED_PTR(Symbol)
    DEFINE_SHARED_PTR(IEvaluatable)

    class IEvaluatable {
    public:
        // specify if expression is calculated
        bool isCalculated = false;
        // type of this symbol (expression)
        TypePtr symbolType = nullptr;
        // token after calculated (after calc)
        TokenPtr calculatedToken = nullptr;

        bool IsCalculated() const;
        void setIsCalculated(bool isCalculated);
        const TypePtr &getSymbolType() const;
        void setSymbolType(const TypePtr &symbolType);
        const TokenPtr &getCalculatedToken() const;
        void setCalculatedToken(const TokenPtr &calculatedToken);

        void copyEvalPerproty(IEvaluatablePtr another);

        virtual void eval() = 0;
    };

    class Symbol {
    public:
        enum Kind {
            SYMBOL = 0,

            // expression
            Expression, commaExpression, typeInfoExpression,
            AssignmentExpression, LogicalOrExpression, ConditionalExpression, LogicalAndExpression, Arith, UNARY, LogicalNot, CastExpression,
            postfixExpression, StructAccess, ArrayAccess, FunctionCall, ArgumentExpressionList, AnonymousArray, PrimaryExpression,

            // declaration
            declaration, declarationSpecifier, storageClassSpecifier, typeSpecifier, typeQualifier, functionSpecifier, alignmentSpecifier,
            structOrUninSpecifier, structDeclarationList, structDeclaration, structDeclaratorList, structDeclarator, enumSpecifier, enumerator,
            declarator, pointer_decl, directDeclarator, parameterTypeList, parameter, parameterDecleartion, parameterList, abstractDeclarator, 
            typeName, directAbstractDeclarator, init, initList, designation, designator, initDeclr

        };

        virtual int getKind() { return Kind::SYMBOL; }

        static bool isPreprocessorSymbol;
    };

}

#endif