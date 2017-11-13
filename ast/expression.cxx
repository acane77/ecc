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
        assert( ( tok->is(Tag::Number) || tok->is(Tag::Identifier) || tok->is(Tag::Constant) ) && "tok is not a primary expression" );
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

    TokenPtr Expression::calculateConstant(TokenPtr tok1, TokenPtr tok2, TokenPtr op) {
        // calculate constant
        // type conversion (normal):                 (relationship)
        //         int   float  char                 all to int
        //   int   int   float  int
        //   float float float  float
        //   char  int   float  int

        if ( op->isRelationshipOperators() ) {
            // Relationship operator
            if ( tok1->is(Tag::Floating) || tok2->is(Tag::Floating) ) {
                // If any one is Float the result is Float
                return make_shared<IntToken>(calculateCompare( tok1->toFloat() , tok2->toFloat(), op->tag ));
            }
            else /*if ( tok1->is(Tag::Integer) || tok2->is(Tag::Integer) )*/ {
                // any of it is integer
                return make_shared<IntToken>(calculateCompare( tok1->toInt() , tok2->toInt(), op->tag ));
            }
        }
        // normal type
        if ( tok1->is(Tag::Floating) || tok2->is(Tag::Floating) ) {
            // If any one is Float the result is Float
            FloatingType result = calculateConstantValue( tok1->toFloat() , tok2->toFloat(), op->tag );
            return make_shared<FloatToken>(result, 64);
        }
        else  /*if ( tok1->is(Tag::Integer) || tok2->is(Tag::Integer) )*/ {
            // any of it is integer
            IntegerType result = calculateConstantValue( tok1->toInt() , tok2->toInt(), op->tag );
            return make_shared<IntToken>(result);
        }
    }

    // TODO:  and replace tree node

    bool Expression::isCalculatable(TokenPtr tok1, TokenPtr tok2, int op, bool calculateRelationship) {
        if ( ( tok2->is(Tag::Floating) && ( tok1->is(Tag::Number) || tok1->is(Tag::Character) ) ) ||
                ( tok1->is(Tag::Floating) && ( tok2->is(Tag::Number) || tok2->is(Tag::Character) ) ) ) {
            if ( op == '*' || op == '/' || op == '+' || op == '-' || op == ',' )
                return true;
            else if ( calculateRelationship && ( op == Tag::And || op == Tag::Or ) )
                return true;
        }
        else if ( (tok1->is(Tag::Integer) || tok1->is(Tag::Character) ) && ( tok2->is(Tag::Integer) || tok2->is(Tag::Character) ) ) {
            if ( op == '*' || op == '/' || op == '+' || op == '-' || op == '%' || op == Tag::LeftShift || op == Tag::RightShift || op == ',' )
                return true;
            else if ( calculateRelationship && ( op == Tag::And || op == Tag::Or ) )
                return true;
        }
        return false;

        /*return ( ( ( tok2->is(Tag::Floating) && ( tok1->is(Tag::Number) || tok1->is(Tag::Character) ) ) ||
                   ( tok1->is(Tag::Floating) && ( tok2->is(Tag::Number) || tok2->is(Tag::Character) ) ) ) &&
                 ( ( op == '*' || op == '/' || op == '+' || op == '-' || op == ',' )
                    || (calculateRelationship && ( op == Tag::And || op == Tag::Or ) ) ) ) ||
                ( ( (tok1->is(Tag::Integer) || tok1->is(Tag::Character) ) && ( tok2->is(Tag::Integer) || tok2->is(Tag::Character) )  ) &&
                    ( op == '*' || op == '/' || op == '+' || op == '-' || op == ','
                      || op == '%' || op == Tag::LeftShift || op == Tag::RightShift ||
                            ( calculateRelationship && ( op == Tag::And || op == Tag::Or ) ) ) );*/

    }

    FloatingType Expression::calculateConstantValue(FloatingType a, FloatingType b, int op) {
        switch (op) {
            case '*': return a * b;
            case '/': return a / b;
            case '+': return a + b;
            case '-': return a - b;
            case ',': return b;

            // the following is for preprocessing only
            case Tag::And: return a && b;
            case Tag::Or:  return a || b;
        }
        assert( false && "not calculatable" );
    }

    IntegerType Expression::calculateConstantValue(IntegerType a, IntegerType b, int op) {
        switch (op) {
            case '*': return a * b;
            case '/': return a / b;
            case '+': return a + b;
            case '-': return a - b;
            case '%': return a % b;
            case Tag::LeftShift: return a << b;
            case Tag::RightShift: return a >> b;
            case '&': return a & b;
            case '|': return a | b;
            case '^': return a ^ b;
            case ',': return b;

            // the following is for preprocessing only
            case Tag::And: return a && b;
            case Tag::Or:  return a || b;
        }
        assert( false && "not calculatable" );
    }

    IntegerType Expression::calculateCompare(FloatingType a, FloatingType b, int op) {
        switch (op) {
            case '<': return a<b;
            case '>': return a>b;
            case Tag::LessThanEqual: return a<=b;
            case Tag::GreaterThanEqual: return a >= b;
            case Tag::Equal: return a == b;
            case Tag::NotEqual: return a!=b;
        }
        assert( false && "not calculatable" );
    }

    IntegerType Expression::calculateCompare(IntegerType a, IntegerType b, int op) {
        switch (op) {
            case '<': return a<b;
            case '>': return a>b;
            case Tag::LessThanEqual: return a<=b;
            case Tag::GreaterThanEqual: return a >= b;
            case Tag::Equal: return a == b;
            case Tag::NotEqual: return a!=b;
        }
        assert( false && "not calculatable" );
    }

    TokenPtr Expression::calculateConstant(TokenPtr tok, TokenPtr op) {
        // calculator constant for unary
        //  return itself
        if ( op->is('!') ) {
            if ( tok->is(Tag::Floating) ) {
                return make_shared<IntToken>( !tok->toFloat() );
            }
            else {  // is integer ot character
                return make_shared<IntToken>( !tok->toInt() );
            }
        }
        if ( tok->is(Tag::Floating) ) {
            return make_shared<FloatToken>(calculateConstantValue( tok->toFloat(), op->tag ), 64);
        }
        else {  // is integer ot character
            return make_shared<IntToken>(calculateConstantValue( tok->toInt(), op->tag ));
        }
        return Miyuki::Lex::TokenPtr();
    }

    FloatingType Expression::calculateConstantValue(FloatingType a, int op) {
        switch (op) {
            case '+': return a;
            case '-': return -a;
        }
        assert( false && "not calculatable" );
    }

    IntegerType Expression::calculateConstantValue(IntegerType a, int op) {
        switch (op) {
            case '~': return ~a;
            case '+': return a;
            case '-': return -a;
        }
        assert( false && "not calculatable" );
    }

    bool Expression::isCalculatable(TokenPtr tok, int op, bool calculateRelationship) {
        if ( tok->is(Tag::Integer) || tok->is(Tag::Character) ) {
            if ( tok->is('~') || tok->is('+') || tok->is('-') ) {
                return false;
            }
            else if ( calculateRelationship && tok->is('!') )
                return true;
        }
        else if ( tok->is(Tag::Floating) ) {
            if ( tok->is('+') || tok->is('-') ) {
                return false;
            }
            else if ( calculateRelationship && tok->is('!') )
                return true;
        }
        return false;
    }
}