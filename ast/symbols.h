#ifndef _MIYUKI_SYMBOLS_H
#define _MIYUKI_SYMBOLS_H

#include "lex/token.h"
#include "common/ptrdef.h"
#include "ast/type.h"
#include "llvm/IR/Value.h"

namespace Miyuki::AST {

// define a list of some type
#ifndef DEFINE_LIST
    #define DEFINE_LIST(type) typedef deque<type##Ptr> type##List; typedef shared_ptr<type##List> type##ListPtr;
#endif // DEFINE_LIST

    using namespace Miyuki::Lex;

    DEFINE_SHARED_PTR(Symbol)
    DEFINE_SHARED_PTR(IEvaluatable)

    class IEvaluatable {
    public:
        // specify if expression is calculated
        bool isCalculated = false;
        // token after calculated (after calc)
        TokenPtr calculatedToken = nullptr;
        // type of this symbol
        TypePtr symbolType = nullptr;
        // temporary address
        Value* addr = nullptr;

        bool IsCalculated() const;
        void setIsCalculated(bool isCalculated);
        const TokenPtr &getCalculatedToken() const;
        void setCalculatedToken(const TokenPtr &calculatedToken);

        TypePtr getSymbolType();
        void setSymbolType(TypePtr type);

        void copyEvalPerproty(IEvaluatablePtr another);

        virtual void eval() = 0;

        // Set address of expression, and its type(?)
        void setAddr(Value* addr);
    };

    class Symbol : public std::enable_shared_from_this<Symbol> {
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
            typeName, directAbstractDeclarator, init, initList, designation, designator, initDeclr,

            // statements
            statement, labeledStatement, compoundStatement, blockItem, expressionStatement, _if, _else, _switch, _while, doWile, _for, _goto, _continue,
            _break, _return,

            // external
            translationUnit, externalDeclaration, functionDefinition

        };

        virtual int getKind() { return Kind::SYMBOL; }

        static bool isPreprocessorSymbol;
    };

}

#endif