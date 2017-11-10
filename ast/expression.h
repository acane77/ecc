#ifndef _MIYUKI_EXPRESSION_H
#define _MIYUKI_EXPRESSION_H

#include "symbols.h"

// ISO/IEC 9899:201x §A.2.1

namespace Miyuki::AST {

    DEFINE_SHARED_PTR(Expression)
    DEFINE_SHARED_PTR(AssignmentExpression)
    DEFINE_SHARED_PTR(ConditionalExpression)
    DEFINE_SHARED_PTR(LogicalORExpression)
    DEFINE_SHARED_PTR(LogicalANDExpression)
    DEFINE_SHARED_PTR(Arithmetic)
    DEFINE_SHARED_PTR(Unary)
    DEFINE_SHARED_PTR(CastExpression)
    DEFINE_SHARED_PTR(LogicalNot)
    DEFINE_SHARED_PTR(PostfixExpression)
    DEFINE_SHARED_PTR(ArrayAccess)
    DEFINE_SHARED_PTR(StructAccess)
    DEFINE_SHARED_PTR(FunctionCall)
    DEFINE_SHARED_PTR(ArgumentExpressionList)
    DEFINE_SHARED_PTR(AnonymousArray)
    DEFINE_SHARED_PTR(PrimaryExpression)

    // NON-EXPRESSION
    DEFINE_SHARED_PTR(TypeName)
    DEFINE_SHARED_PTR(InitializerList)

    class Expression : public Symbol, public IEvaluatable {
    public:
        virtual int getKind() { return Kind::Expression; }
        virtual void gen() = 0;
    };

    class AssignmentExpression : public Expression {
    public:
        TokenPtr assignOp;
        UnaryPtr unaryExp;
        AssignmentExpressionPtr assignExp;
        // OR
        ConditionalExpressionPtr condExp;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::AssignmentExpression; }
    };

    class ConditionalExpression : public Expression {
    public:
        LogicalORExpressionPtr   logicalOrExp;
        ExpressionPtr            exp;
        ConditionalExpressionPtr condExpr;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::ConditionalExpression; }
    };

    class LogicalORExpression : public Expression {
    public:
        LogicalANDExpressionPtr logicalAndExp;
        LogicalORExpressionPtr logicalOrExp;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::LogicalOrExpression; }
    };

    class LogicalANDExpression : public Expression {
    public:
        ArithmeticPtr exclusiveOrExpression;
        LogicalANDExpressionPtr logicalAndExpression;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::LogicalAndExpression; }
    };

    class Arithmetic : public Expression {
    public:
        TokenPtr      op;
        ExpressionPtr expr1;
        ExpressionPtr expr2;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::Arith; }
    };

    class Unary : public Expression {
    public:
        TokenPtr      op;
        ExpressionPtr expr;

        virtual void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::Unary; }
    };

    class CastExpression : public Unary {
    public:
        TypeNamePtr  typeName;

        virtual void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::CastExpression; }
    };

    class LogicalNot : public Unary {
    public:
        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::LogicalNot; }
    };

    typedef ConditionalExpression ConstantExpression;
    typedef shared_ptr<ConstantExpression> ConstantExpressionPtr;

    class PostfixExpression : public Unary {
    public:
        TokenPtr op;
        PostfixExpressionPtr postfixExp;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::PostfixExpression; }
    };

    class StructAccess : public PostfixExpression {
    public:
        WordTokenPtr identifier;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::StructAccess; }
    };

    class ArrayAccess : public PostfixExpression {
    public:
        ExpressionPtr exp;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::ArrayAccess; }
    };

    class FunctionCall : public PostfixExpression {
    public:
        ArgumentExpressionListPtr argExprLst;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::FunctionCall; }
    };

    class ArgumentExpressionList : public Expression {
    public:
        ArgumentExpressionListPtr argExprLst;
        AssignmentExpressionPtr assignExpr;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::ArgumentExpressionList; }
    };

    class AnonymousArray : public Expression {
        TypeNamePtr typeName;
        InitializerListPtr initList;
        // TODO: implement this after implement Statement (if there are repeat code, try def)
        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::AnonymousArray; }
    };

    class PrimaryExpression : public Expression {
    public:
        TokenPtr factor;
        // OR
        ExpressionPtr exp;

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::PrimaryExpression; }
    };

}

#endif