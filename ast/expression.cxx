#include "expression.h"

namespace Miyuki::AST {

    // constructors
    AssignmentExpression::AssignmentExpression(const TokenPtr &assignOp, const ConditionalExpressionPtr &condExp,
                                               const AssignmentExpressionPtr &assignExp) : assignOp(assignOp),
                                                                                           condExp(condExp),
                                                                                           assignExp(assignExp) {}

    AssignmentExpression::AssignmentExpression(const ConditionalExpressionPtr &condExp) : condExp(condExp) {}

    ConditionalExpression::ConditionalExpression(const LogicalORExpressionPtr &logicalOrExp, const ExpressionPtr &exp,
                                                 const ConditionalExpressionPtr &condExpr) : logicalOrExp(logicalOrExp),
                                                                                             exp(exp),
                                                                                             condExpr(condExpr) {}

    LogicalORExpression::LogicalORExpression(const LogicalANDExpressionPtr &logicalAndExp,
                                             const LogicalORExpressionPtr &logicalOrExp) : logicalAndExp(logicalAndExp),
                                                                                           logicalOrExp(logicalOrExp) {}

    LogicalANDExpression::LogicalANDExpression(const ArithmeticPtr &exclusiveOrExpression,
                                               const LogicalANDExpressionPtr &logicalAndExpression)
            : exclusiveOrExpression(exclusiveOrExpression), logicalAndExpression(logicalAndExpression) {}

    Arithmetic::Arithmetic(const TokenPtr &op, const ExpressionPtr &expr1, const ExpressionPtr &expr2) : op(op), expr1(expr1), expr2(expr2) {}

    Unary::Unary(const TokenPtr &op, const ExpressionPtr &expr) : op(op), expr(expr) {}

    StructAccess::StructAccess(const TokenPtr &op, const ExpressionPtr &expr,
                               const PostfixExpressionPtr &postfixExp, const WordTokenPtr &identifier)
            : PostfixExpression(op, expr, postfixExp), identifier(identifier) {}

    ArrayAccess::ArrayAccess(const TokenPtr &op, const ExpressionPtr &expr,
                             const PostfixExpressionPtr &postfixExp, const ExpressionPtr &exp) : PostfixExpression(op, expr, postfixExp),
                                                                                                 exp(exp) {}

    FunctionCall::FunctionCall(const TokenPtr &op, const ExpressionPtr &expr,
                               const PostfixExpressionPtr &postfixExp, const ArgumentExpressionListPtr &argExprLst)
            : PostfixExpression(op, expr, postfixExp), argExprLst(argExprLst) {}

    ArgumentExpressionList::ArgumentExpressionList(const ArgumentExpressionListPtr &argExprLst,
                                                   const AssignmentExpressionPtr &assignExpr)
            : argExprLst(argExprLst), assignExpr(assignExpr) {}

    AnonymousArray::AnonymousArray(const TypeNamePtr &typeName, const InitializerListPtr &initList)
            : typeName(typeName), initList(initList), PostfixExpression(nullptr, nullptr, nullptr) {}

    PrimaryExpression::PrimaryExpression(const TokenPtr &factor) : factor(factor), PostfixExpression(nullptr, nullptr, nullptr) {}

    PrimaryExpression::PrimaryExpression(const ExpressionPtr &exp) : exp(exp), PostfixExpression(nullptr, nullptr, nullptr) {}

    ExpressionPtr ExpressionBuilder::getSymbol(TypePtr typ, TokenPtr tok) {
        // this function is for calculated value, so only return calculatable primary-expressions
        PrimaryExpressionPtr expr = make_shared<PrimaryExpression>(tok);
        expr->setSymbolType(typ);
        return expr;
    }

    CommaExpression::CommaExpression(const CommaExpressionPtr &commaExp, const AssignmentExpressionPtr &assignExp)
            : commaExp(commaExp), assignExp(assignExp) {}

    CastExpression::CastExpression(const TypeNamePtr &typeName, const CastExpressionPtr &castExpr)
            : typeName(typeName), castExpr(castExpr) {}

    CastExpression::CastExpression(const UnaryPtr &unaryExpr) : unaryExpr(unaryExpr) {}

    TypeInfoExpression::TypeInfoExpression(const TokenPtr &op, const TypeNamePtr &typeName)
            : Unary(op, nullptr), typeName(typeName) {}
}