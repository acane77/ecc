#ifndef _MIYUKI_EXPRESSION_H
#define _MIYUKI_EXPRESSION_H

#include "symbols.h"

// ISO/IEC 9899:201x §A.2.1

namespace Miyuki::AST {

    DEFINE_SHARED_PTR(Expression)
    DEFINE_SHARED_PTR(CommaExpression)
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
    DEFINE_SHARED_PTR(TypeInfoExpression)

    // NON-EXPRESSION (move to other place after implement)
    DEFINE_SHARED_PTR(TypeName)
    DEFINE_SHARED_PTR(InitializerList)

    class Expression : public Symbol, public IEvaluatable {
    public:
        Expression() {}

        virtual int getKind() { return Kind::Expression; }
        virtual void gen() = 0;
        virtual void eval() { assert( false && "unimplemented" ); }
    };

    class ExpressionBuilder {
    public:
        static ExpressionPtr getSymbol(TypePtr typ, TokenPtr tok);
    };

    class CommaExpression : public Expression {
    public:
        CommaExpressionPtr commaExp;
        AssignmentExpressionPtr assignExp;

        CommaExpression(const CommaExpressionPtr &commaExp, const AssignmentExpressionPtr &assignExp);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::commaExpression; }
    };

    class AssignmentExpression : public Expression {
    public:
        TokenPtr assignOp;
        ConditionalExpressionPtr condExp;
        AssignmentExpressionPtr assignExp;

        AssignmentExpression(const TokenPtr &assignOp, const ConditionalExpressionPtr &condExp,
                             const AssignmentExpressionPtr &assignExp);
        AssignmentExpression(const ConditionalExpressionPtr &condExp);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::AssignmentExpression; }
    };

    class ConditionalExpression : public Expression {
    public:
        LogicalORExpressionPtr   logicalOrExp;
        ExpressionPtr            exp;
        ConditionalExpressionPtr condExpr;

        ConditionalExpression(const LogicalORExpressionPtr &logicalOrExp, const ExpressionPtr &exp = nullptr,
                              const ConditionalExpressionPtr &condExpr = nullptr);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::ConditionalExpression; }
    };

    class LogicalORExpression : public Expression {
    public:
        LogicalANDExpressionPtr logicalAndExp;
        LogicalORExpressionPtr logicalOrExp;

        LogicalORExpression(const LogicalANDExpressionPtr &logicalAndExp, const LogicalORExpressionPtr &logicalOrExp);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::LogicalOrExpression; }
    };

    class LogicalANDExpression : public Expression {
    public:
        ArithmeticPtr exclusiveOrExpression;
        LogicalANDExpressionPtr logicalAndExpression;

        LogicalANDExpression(const ArithmeticPtr &exclusiveOrExpression,
                             const LogicalANDExpressionPtr &logicalAndExpression);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::LogicalAndExpression; }
    };

    class Arithmetic : public Expression {
    public:
        TokenPtr      op;
        ExpressionPtr expr1;
        ExpressionPtr expr2;

        Arithmetic(const TokenPtr &op, const ExpressionPtr &expr1, const ExpressionPtr &expr2);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::Arith; }
    };

    class Unary : public Expression {
    public:
        TokenPtr      op;
        ExpressionPtr expr;

        Unary(const TokenPtr &op, const ExpressionPtr &expr);

        virtual void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::UNARY; }
    };

    class CastExpression : public Expression {
    public:
        TypeNamePtr  typeName;
        CastExpressionPtr castExpr;
        // OR
        UnaryPtr     unaryExpr;

        CastExpression(const UnaryPtr &unaryExpr);
        CastExpression(const TypeNamePtr &typeName, const CastExpressionPtr &castExpr);

        virtual void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::CastExpression; }
    };

    class LogicalNot : public Unary {
    public:
        // we use original token because original token contains more information
        // eg token position
        LogicalNot(const TokenPtr &op, const ExpressionPtr &expr) : Unary(op, expr) {}

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::LogicalNot; }
    };

    class TypeInfoExpression : public Unary {
    public:
        TypeNamePtr typeName;

        TypeInfoExpression(const TokenPtr &op, const TypeNamePtr &typeName);
        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::typeInfoExpression; }
    };

    typedef ConditionalExpression ConstantExpression;
    typedef shared_ptr<ConstantExpression> ConstantExpressionPtr;

    class PostfixExpression : public Unary {
    public:
        PostfixExpressionPtr postfixExp;

        PostfixExpression(const TokenPtr &op, const ExpressionPtr &expr, const PostfixExpressionPtr &postfixExp)
                : Unary(op, expr), postfixExp(postfixExp) {}

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::postfixExpression; }
    };

    class StructAccess : public PostfixExpression {
    public:
        WordTokenPtr identifier;

        StructAccess(const TokenPtr &op, const ExpressionPtr &expr,
                     const PostfixExpressionPtr &postfixExp, const WordTokenPtr &identifier);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::StructAccess; }
    };

    class ArrayAccess : public PostfixExpression {
    public:
        ExpressionPtr exp;

        ArrayAccess(const TokenPtr &op, const ExpressionPtr &expr,
                    const PostfixExpressionPtr &postfixExp, const ExpressionPtr &exp);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::ArrayAccess; }
    };

    class FunctionCall : public PostfixExpression {
    public:
        ArgumentExpressionListPtr argExprLst;

        FunctionCall(const TokenPtr &op, const ExpressionPtr &expr,
                     const PostfixExpressionPtr &postfixExp, const ArgumentExpressionListPtr &argExprLst);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::FunctionCall; }
    };

    class ArgumentExpressionList : public Expression {
    public:
        ArgumentExpressionListPtr argExprLst;
        AssignmentExpressionPtr assignExpr;

        ArgumentExpressionList(const ArgumentExpressionListPtr &argExprLst, const AssignmentExpressionPtr &assignExpr);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::ArgumentExpressionList; }
    };

    class AnonymousArray : public PostfixExpression {
    public:
        TypeNamePtr typeName;
        AnonymousArray(const TypeNamePtr &typeName, const InitializerListPtr &initList);

        InitializerListPtr initList;
        // TODO: implement this after implement Statement (if there are repeat code, try def)
        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::AnonymousArray; }
    };

    class PrimaryExpression : public PostfixExpression {
    public:
        TokenPtr factor;
        // OR
        ExpressionPtr exp;

        PrimaryExpression(const ExpressionPtr &exp);
        PrimaryExpression(const TokenPtr &factor);

        void gen() { assert( false && "unimplemented" ); }
        int getKind() { return Kind::PrimaryExpression; }
    };

}

#endif