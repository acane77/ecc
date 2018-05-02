#ifndef _MIYUKI_EXPRESSION_H
#define _MIYUKI_EXPRESSION_H

#include "symbols.h"
#include "common/md5.h"

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

        // for evaled child tokens replace them, by every subclass itself
        //   first test if child token is calculated, and if calculated, replace it by modifying pointer
        //   get symbol from token using ExpressionBuilder
        // this function is called by eval(), AFTER eval, try replace
        // replace all token into primary-expression
        virtual void replaceCalculatedChild() { assert( false && "unimplemented" ); }

        // for arithmetic
        TokenPtr calculateConstant(TokenPtr tok1, TokenPtr tok2, TokenPtr op);
        FloatingLiteralType calculateConstantValue(FloatingLiteralType a, FloatingLiteralType b, int op);
        IntegerLiteralType  calculateConstantValue(IntegerLiteralType  a, IntegerLiteralType b, int op);
        IntegerLiteralType  calculateCompare(FloatingLiteralType a, FloatingLiteralType b, int op);
        IntegerLiteralType  calculateCompare(IntegerLiteralType a, IntegerLiteralType b, int op);
        bool isCalculatable(TokenPtr tok1, TokenPtr tok2, int op, bool calculateRelationship);

        // for unary
        TokenPtr calculateConstant(TokenPtr tok, TokenPtr op);
        FloatingLiteralType calculateConstantValue(FloatingLiteralType a, int op);
        IntegerLiteralType  calculateConstantValue(IntegerLiteralType a, int op);
        bool isCalculatable(TokenPtr tok, int op, bool calculateRelationship);
    };

    class CommaExpression : public Expression {
    public:
        CommaExpressionPtr commaExp;
        AssignmentExpressionPtr assignExp;

        CommaExpression(const CommaExpressionPtr &commaExp, const AssignmentExpressionPtr &assignExp);

        void gen();
        int getKind() { return Kind::commaExpression; }

        virtual void eval();
    };

    class AssignmentExpression : public Expression {
    public:
        TokenPtr assignOp;
        ConditionalExpressionPtr condExp;
        AssignmentExpressionPtr assignExp;

        AssignmentExpression(const TokenPtr &assignOp, const ConditionalExpressionPtr &condExp,
                             const AssignmentExpressionPtr &assignExp);
        AssignmentExpression(const ConditionalExpressionPtr &condExp);

        void gen();
        int getKind() { return Kind::AssignmentExpression; }

        virtual void eval();
    };

    class ConditionalExpression : public Expression {
    public:
        LogicalORExpressionPtr   logicalOrExp;
        ExpressionPtr            exp;
        ConditionalExpressionPtr condExpr;

        ConditionalExpression(const LogicalORExpressionPtr &logicalOrExp, const ExpressionPtr &exp = nullptr,
                              const ConditionalExpressionPtr &condExpr = nullptr);

        void gen();
        int getKind() { return Kind::ConditionalExpression; }

        virtual void eval();
    };

    class LogicalORExpression : public Expression {
    public:
        LogicalANDExpressionPtr logicalAndExp;
        LogicalORExpressionPtr logicalOrExp;

        LogicalORExpression(const LogicalANDExpressionPtr &logicalAndExp, const LogicalORExpressionPtr &logicalOrExp);

        void gen();
        int getKind() { return Kind::LogicalOrExpression; }

        virtual void eval();
    };

    class LogicalANDExpression : public Expression {
    public:
        ArithmeticPtr exclusiveOrExpression;
        LogicalANDExpressionPtr logicalAndExpression;

        LogicalANDExpression(const ArithmeticPtr &exclusiveOrExpression,
                             const LogicalANDExpressionPtr &logicalAndExpression);

        void gen();
        int getKind() { return Kind::LogicalAndExpression; }

        virtual void eval();
    };

    class Arithmetic : public Expression {
    public:
        TokenPtr      op;
        ExpressionPtr expr1;
        ExpressionPtr expr2;

        Arithmetic(const TokenPtr &op, const ExpressionPtr &expr1, const ExpressionPtr &expr2);

        void gen();
        int getKind() { return Kind::Arith; }

        virtual void eval() override;
    };

    class Unary : public Expression {
    public:
        TokenPtr      op;
        ExpressionPtr expr;

        // Check whether the name of the function is addressed (use &)
        bool          isFunctionDesignatorFetchAddr = false;

        Unary(const TokenPtr &op, const ExpressionPtr &expr);

        virtual void gen();
        int getKind() { return Kind::UNARY; }

        virtual void eval() override;
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

        virtual void eval() override;
    };

    class LogicalNot : public Unary {
    public:
        // we use original token because original token contains more information
        // eg token position
        LogicalNot(const TokenPtr &op, const ExpressionPtr &expr) : Unary(op, expr) {}

        void gen();
        int getKind() { return Kind::LogicalNot; }
    };

    class TypeInfoExpression : public Unary {
    public:
        TypeNamePtr typeName;

        TypeInfoExpression(const TokenPtr &op, const TypeNamePtr &typeName);
        void gen();
        int getKind() { return Kind::typeInfoExpression; }
        virtual void eval() override { /* not evaluatable*/ }
    };

    typedef ConditionalExpression ConstantExpression;
    typedef shared_ptr<ConstantExpression> ConstantExpressionPtr;

    class PostfixExpression : public Unary {
    public:
        PostfixExpressionPtr postfixExp;

        PostfixExpression(const TokenPtr &op, const ExpressionPtr &expr, const PostfixExpressionPtr &postfixExp)
                : Unary(op, expr), postfixExp(postfixExp) {}

        void gen();
        int getKind() { return Kind::postfixExpression; }
        virtual void eval() override { /* not evaluatable*/ }
    };

    class StructAccess : public PostfixExpression {
    public:
        WordTokenPtr identifier;

        StructAccess(const TokenPtr &op, const ExpressionPtr &expr,
                     const PostfixExpressionPtr &postfixExp, const WordTokenPtr &identifier);

        void gen();
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
        virtual void eval() override { /* not evaluatable*/ }
    };

    class AnonymousArray : public PostfixExpression {
    public:
        TypeNamePtr typeName;
        AnonymousArray(const TypeNamePtr &typeName, const InitializerListPtr &initList);

        InitializerListPtr initList;
        
        void gen();
        int getKind() { return Kind::AnonymousArray; }
    };

    class PrimaryExpression : public PostfixExpression {
    public:
        TokenPtr factor;
        // OR
        ExpressionPtr exp;

        PrimaryExpression(const ExpressionPtr &exp);
        PrimaryExpression(const TokenPtr &factor);

        void gen();
        int getKind() { return Kind::PrimaryExpression; }
        virtual void eval() override;
    };

}

#endif