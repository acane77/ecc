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
        bool isCalculated;
        TypePtr symbolType;

        const TypePtr &getSymbolType() const {
            return symbolType;
        }

        void setSymbolType(const TypePtr &symbolType) {
            IEvaluatable::symbolType = symbolType;
        }

        virtual void eval() = 0;
    };

    class Symbol {
    public:
        enum Kind {
            SYMBOL = 0,

            Expression,
            AssignmentExpression, LogicalOrExpression, ConditionalExpression, LogicalAndExpression, Arith, UNARY, LogicalNot, CastExpression,
            postfixExpression, StructAccess, ArrayAccess, FunctionCall, ArgumentExpressionList, AnonymousArray, PrimaryExpression
        };

        virtual int getKind() { return Kind::SYMBOL; }
    };

}

#endif