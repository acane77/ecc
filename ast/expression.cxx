#include "expression.h"
#include "parse/ifparser.h"
#include "ast/value.h"
#include "ast/env.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/TypeName.h"

namespace Miyuki::AST {

    using namespace std;

#define EVAL_EXPRESSION(siblingLevel, nextLevel, op) \
    /* if this symbol has already been calculated */\
    if (IsCalculated()) return; \
    if (nextLevel) nextLevel->eval(); /* if next level is exist */ \
    /*  if no same level, directly return */\
    if (!siblingLevel) {\
        /* if no sibling level, copy property of next level. */\
        if (nextLevel) copyEvalPerproty(nextLevel);\
        return;\
    }/* else has same level, eval and try calculate */ \
    siblingLevel->eval();\
    if ( isCalculatable( siblingLevel->getCalculatedToken(), nextLevel->getCalculatedToken() , op->tag , isPreprocessorSymbol) ) {\
        setCalculatedToken( calculateConstant( siblingLevel->getCalculatedToken(), nextLevel->getCalculatedToken(), op ) );\
    }

#define RETURN_IF_CALCULATED() if (IsCalculated()) return
#define TRY_EVAL(expr) if (expr) expr->eval()
#define PRINT_FUNCTION_NAME() cout << "Function:   " << typeid(this).name() << "::" << __FUNCTION__ << "(), at line: " << __FILE__ << ":" << __LINE__ << endl

    // constructors
    AssignmentExpression::AssignmentExpression(const TokenPtr &assignOp, const ConditionalExpressionPtr &condExp,
                                               const AssignmentExpressionPtr &assignExp) : assignOp(assignOp),
                                                                                           condExp(condExp),
                                                                                           assignExp(assignExp) {}

    AssignmentExpression::AssignmentExpression(const ConditionalExpressionPtr &condExp) : condExp(condExp) {}

    void AssignmentExpression::eval() {
        PRINT_FUNCTION_NAME();
        EVAL_EXPRESSION(assignExp, condExp, assignOp);
    }

    ConditionalExpression::ConditionalExpression(const LogicalORExpressionPtr &logicalOrExp, const ExpressionPtr &exp,
                                                 const ConditionalExpressionPtr &condExpr) : logicalOrExp(logicalOrExp),
                                                                                             exp(exp),
                                                                                             condExpr(condExpr) {}

    void ConditionalExpression::eval() {
        PRINT_FUNCTION_NAME();
        RETURN_IF_CALCULATED();
        TRY_EVAL(condExpr);
        TRY_EVAL(exp);
        TRY_EVAL(logicalOrExp);
        
        // if only has next level
        if (!condExpr || !exp) {
            if (logicalOrExp) copyEvalPerproty(logicalOrExp);
            return;
        }

        if ( condExpr && exp && logicalOrExp && condExpr->IsCalculated() && exp->IsCalculated() && logicalOrExp->IsCalculated() ) {
            // test condition
            if (logicalOrExp->getCalculatedToken()->toInt())
                // if condition is true
                setCalculatedToken(exp->getCalculatedToken());
            else
                // condition is false
                setCalculatedToken(condExpr->getCalculatedToken());
        }
    }

    LogicalORExpression::LogicalORExpression(const LogicalANDExpressionPtr &logicalAndExp,
                                             const LogicalORExpressionPtr &logicalOrExp) : logicalAndExp(logicalAndExp),
                                                                                           logicalOrExp(logicalOrExp) {}

    void LogicalORExpression::eval() {
        PRINT_FUNCTION_NAME();
        EVAL_EXPRESSION(logicalOrExp, logicalAndExp, make_shared<Token>(Tag::Or));
    }

    LogicalANDExpression::LogicalANDExpression(const ArithmeticPtr &exclusiveOrExpression,
                                               const LogicalANDExpressionPtr &logicalAndExpression)
            : exclusiveOrExpression(exclusiveOrExpression), logicalAndExpression(logicalAndExpression) {}

    void LogicalANDExpression::eval() {
        PRINT_FUNCTION_NAME();
        EVAL_EXPRESSION(logicalAndExpression, exclusiveOrExpression, make_shared<Token>(Tag::And));
    }

    Arithmetic::Arithmetic(const TokenPtr &op, const ExpressionPtr &expr1, const ExpressionPtr &expr2) : op(op), expr1(expr1), expr2(expr2) {}

    void Arithmetic::eval() {
        PRINT_FUNCTION_NAME();
        //EVAL_EXPRESSION(expr1, expr2, op);
        /* if this symbol has already been calculated */
        ExpressionPtr nextLevel = expr1, siblingLevel = expr2;
        if (IsCalculated()) return;
        if (nextLevel) nextLevel->eval(); /* if next level is exist */
                                          /*  if no same level, directly return */
        if (!siblingLevel) {
            /* if no sibling level, copy property of next level. */
            if (nextLevel) copyEvalPerproty(nextLevel);
            return;
        }/* else has same level, eval and try calculate */
        siblingLevel->eval();
        if (isCalculatable(nextLevel->getCalculatedToken(), siblingLevel->getCalculatedToken(), op->tag, isPreprocessorSymbol)) {
            setCalculatedToken(calculateConstant(nextLevel->getCalculatedToken(), siblingLevel->getCalculatedToken(), op));
        }
    }

    Unary::Unary(const TokenPtr &op, const ExpressionPtr &expr) : op(op), expr(expr) {}

    void Unary::eval() {
        PRINT_FUNCTION_NAME();
        RETURN_IF_CALCULATED();
        TRY_EVAL(expr);
        if (expr && expr->IsCalculated()) {
            // if is calculated
            setCalculatedToken(calculateConstant(expr->getCalculatedToken(), op));
        }
        else if (!expr)
            assert(false && "invalid unary (no expression)");
    }

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

    PrimaryExpression::PrimaryExpression(const TokenPtr &factor) : factor(factor), PostfixExpression(nullptr, nullptr, nullptr) {
        setSymbolType( TypeFactory::build(factor) );
    }

    PrimaryExpression::PrimaryExpression(const ExpressionPtr &exp) : exp(exp), PostfixExpression(nullptr, nullptr, nullptr) {
        setSymbolType( exp->getSymbolType() );
    }

    void PrimaryExpression::eval() {
        PRINT_FUNCTION_NAME();
        // if is factor, and is number or character
        // not all factor is calculated
        if (factor && ( factor->is(Tag::Number) || factor->is(Tag::Character) ) ) {
            setCalculatedToken(factor);
        }
        // else is ( expression ) or other factors
        else {
            TRY_EVAL(exp);
            if (exp && exp->IsCalculated())
                setCalculatedToken(exp->getCalculatedToken());
        }
    }

    CommaExpression::CommaExpression(const CommaExpressionPtr &commaExp, const AssignmentExpressionPtr &assignExp)
            : commaExp(commaExp), assignExp(assignExp) {}

    void CommaExpression::eval() {
        EVAL_EXPRESSION(commaExp, assignExp, make_shared<Token>(','));
    }

    CastExpression::CastExpression(const TypeNamePtr &typeName, const CastExpressionPtr &castExpr)
            : typeName(typeName), castExpr(castExpr) {}

    void CastExpression::eval() {
        PRINT_FUNCTION_NAME();
        if (unaryExpr) {
            unaryExpr->eval();
            copyEvalPerproty(unaryExpr);
        }
    }

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
            FloatingLiteralType result = calculateConstantValue( tok1->toFloat() , tok2->toFloat(), op->tag );
            return make_shared<FloatToken>(result, 64);
        }
        else  /*if ( tok1->is(Tag::Integer) || tok2->is(Tag::Integer) )*/ {
            // any of it is integer
            IntegerLiteralType result = calculateConstantValue( tok1->toInt() , tok2->toInt(), op->tag );
            return make_shared<IntToken>(result);
        }
    }

    // TODO:  and replace tree node

    bool Expression::isCalculatable(TokenPtr tok1, TokenPtr tok2, int op, bool calculateRelationship) {
        if ( tok1 == nullptr || tok2 == nullptr ) return false;

        // if any token is float
        if ( ( tok2->is(Tag::Floating) && ( tok1->is(Tag::Number) || tok1->is(Tag::Character) ) ) ||
                ( tok1->is(Tag::Floating) && ( tok2->is(Tag::Number) || tok2->is(Tag::Character) ) ) ) {
            if ( op == '*' || op == '/' || op == '+' || op == '-' || op == '>' || op == '<' || op == Tag::GreaterThanEqual || op == Tag::LessThanEqual || op == Tag::Equal || op == Tag::NotEqual)
                return true;
            else if ( calculateRelationship && ( op == Tag::And || op == Tag::Or ) )
                return true;
        }
        // if any token is integer
        else if ( (tok1->is(Tag::Integer) || tok1->is(Tag::Character) ) && ( tok2->is(Tag::Integer) || tok2->is(Tag::Character) ) ) {
            if ( op == '*' || op == '/' || op == '+' || op == '-' || op == '%' || op == Tag::LeftShift || op == Tag::RightShift || op == '>' || op == '<' || op == Tag::GreaterThanEqual || op == Tag::LessThanEqual || op == Tag::Equal || op == Tag::NotEqual)
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

    FloatingLiteralType Expression::calculateConstantValue(FloatingLiteralType a, FloatingLiteralType b, int op) {
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

    IntegerLiteralType Expression::calculateConstantValue(IntegerLiteralType a, IntegerLiteralType b, int op) {
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

            // the following is for preprocessing only
            case Tag::And: return a && b;
            case Tag::Or:  return a || b;
        }
        assert( false && "not calculatable" );
    }

    IntegerLiteralType Expression::calculateCompare(FloatingLiteralType a, FloatingLiteralType b, int op) {
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

    IntegerLiteralType Expression::calculateCompare(IntegerLiteralType a, IntegerLiteralType b, int op) {
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

    FloatingLiteralType Expression::calculateConstantValue(FloatingLiteralType a, int op) {
        switch (op) {
            case '+': return a;
            case '-': return -a;
        }
        assert( false && "not calculatable" );
    }

    IntegerLiteralType Expression::calculateConstantValue(IntegerLiteralType a, int op) {
        switch (op) {
            case '~': return ~a;
            case '+': return a;
            case '-': return -a;
        }
        assert( false && "not calculatable" );
    }

    bool Expression::isCalculatable(TokenPtr tok, int op, bool calculateRelationship) {
        if ( tok == nullptr ) return false;
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

    //////////////// Code Generation //////////////////
    /// NOTE: Call eval() before call gen()
    void codeGen(string code) {
        cout << "[CodeGen]\t" << code << "\n";
    }

    Value* errorValue(const string& msg, TokenPtr errTok, bool throwError = false) {
        Parse::IParser::instance->diagError(string(msg), errTok);
        if (throwError) throw exception(msg.c_str());
        return nullptr;
    }

#define TRY_GEN(x, ...) if(x) x->gen(__VA_ARGS__)
#define ReturnAddressIfCalculated() \
    if (IsCalculated()) {\
        setSymbolType(TypeFactory::build(getCalculatedToken()));\
        addr = ValueFactory::build(getCalculatedToken());\
        return;\
    }
#define SetExpressLevel(next, sibling) ExpressionPtr nextLevel = next, siblingLevel = sibling
#define InheritSymbolTypeFrom(x) setSymbolType(x->getSymbolType());
#define NewBasicBlock(name) BasicBlock::Create(getGlobalContext(), name)
#define SetInsertBlock(BB) switchBasicBlock(BB)
#define DefineBasicBlock(name) BasicBlock* BB_##name= NewBasicBlock(#name)
#define GetIntNType(N) Type::getInt##N##Ty(getGlobalContext())

    Value* ZeroValue = ConstantInt::getIntegerValue(Type::getInt32Ty(getGlobalContext()), APInt(32, (uint64_t)0));

    void CommaExpression::gen() {
        ReturnAddressIfCalculated();
        SetExpressLevel(assignExp, commaExp);
        // Code Generation
        TRY_GEN(siblingLevel);
        TRY_GEN(nextLevel);
        // set symbol type & addr
        if (siblingLevel) {
            addr = siblingLevel->addr;
            InheritSymbolTypeFrom(siblingLevel);
        }
        else {
            addr = nextLevel->addr;
            InheritSymbolTypeFrom(nextLevel);
        }
    }

    void ConditionalExpression::gen() {
        ReturnAddressIfCalculated();
        // Code generation
        logicalOrExp->gen();

        if (exp && condExpr) {
            DefineBasicBlock(CondExpr_lhs);
            DefineBasicBlock(CondExpr_rhs);
            DefineBasicBlock(CondExpr_end);

            Builder.CreateCondBr(logicalOrExp->addr, BB_CondExpr_lhs, BB_CondExpr_rhs);
            // LHS
            SetInsertBlock(BB_CondExpr_lhs);
            exp->gen();
            Builder.CreateBr(BB_CondExpr_end);

            // rhs
            SetInsertBlock(BB_CondExpr_rhs);
            condExpr->gen();
            Builder.CreateBr(BB_CondExpr_end);

            // end
            SetInsertBlock(BB_CondExpr_end);
        }
        else {
            addr = logicalOrExp->addr;
        }
        /// TODO: implement after JUMP implemented
        //return logicalOrExp->gen(NewBasicBlock("condExpr"));
    }

#define IntermediateCode(x)
    void LogicalORExpression::gen() {
        ReturnAddressIfCalculated();
        // Code generation
        // Action in logical-or
        //   calculate value, if in cond expr, branch
        if (logicalOrExp) {
            // is a || b
            // Left-hand Statement
            BasicBlock * BB_entry = getCurrentBasicBlock();

            DefineBasicBlock(LOR_rhs);
            DefineBasicBlock(LOR_end);
            logicalAndExp->gen();
            Value * toBool = Builder.CreateICmpNE(logicalAndExp->addr, ZeroValue, "toBool");
            Builder.CreateCondBr(toBool, BB_LOR_end, BB_LOR_rhs);
            
            // Right-hand Statement
            SetInsertBlock(BB_LOR_rhs);
            logicalOrExp->gen();
            Value* toBool1 = Builder.CreateICmpNE(logicalOrExp->addr, ZeroValue, "toBool");
            Builder.CreateBr(BB_LOR_end);

            // End
            SetInsertBlock(BB_LOR_end);
            PHINode* phi = Builder.CreatePHI(Type::getInt8Ty(getGlobalContext()), 2, "");
            phi->addIncoming(ConstantInt::getTrue(Type::getInt1Ty(getGlobalContext())), BB_entry);
            phi->addIncoming(toBool1, BB_LOR_rhs);
            
            addr = Builder.CreateZExt(phi, GetIntNType(32), "landExt");
        }
        else {
            addr = logicalAndExp->addr;
        }
    }

    void LogicalANDExpression::gen() {
        ReturnAddressIfCalculated();
        // Codegen
        if (logicalAndExpression) {
            // is a && b
            BasicBlock * BB_entry = getCurrentBasicBlock();

            DefineBasicBlock(LAND_rhs);
            DefineBasicBlock(LAND_end);
            exclusiveOrExpression->gen();

            IntermediateCode({
                toBool = [addr] != 0
                if toBool goto LAND_rhs
                goto LAND_end
      LAND_rhs: gen[logicalAndExpr]
                toBool1 = [logicalAndExpr.addr] != 0
                goto LAND_end
      LAND_end: t1 = if [entry] false elif LAND_ths toBool1 
                [addr] = t1
            });

            Value * toBool = Builder.CreateICmpNE(exclusiveOrExpression->addr, ZeroValue, "toBool");
            
            Builder.CreateCondBr(toBool, BB_LAND_rhs, BB_LAND_end);

            // Right-hand Statement
            SetInsertBlock(BB_LAND_rhs);
            logicalAndExpression->gen();
            Value* toBool1 = Builder.CreateICmpNE(logicalAndExpression->addr, ZeroValue, "toBool");
            Builder.CreateBr(BB_LAND_end);

            // End
            SetInsertBlock(BB_LAND_end);
            PHINode* phi = Builder.CreatePHI(Type::getInt8Ty(getGlobalContext()), 2, "");
            phi->addIncoming(ConstantInt::getFalse(Type::getInt1Ty(getGlobalContext())), BB_entry);
            phi->addIncoming(toBool1, BB_LAND_rhs);

            addr = Builder.CreateZExt(phi, GetIntNType(32), "landExt");
        }
        else {
            addr = exclusiveOrExpression->addr;
        }
    }

    void Arithmetic::gen() {
        ReturnAddressIfCalculated();
        expr1->gen();
        if (expr2 == nullptr) {
            addr = expr1->addr;
            return;
        }
        expr2->gen();

        // GET LHS and RHS
        Value *LHS = expr1->addr, *RHS = expr2->addr;
        // Code Below ensures they are with same type
        // if LHS or RHS is pointer, cast to integer
        if (LHS->getType()->isPointerTy()) {
            LHS = Builder.CreatePointerBitCastOrAddrSpaceCast(LHS, GetIntNType(32), false, "expr_castPtr2Int");
        }
        if (RHS->getType()->isPointerTy()) {
            RHS = Builder.CreatePointerBitCastOrAddrSpaceCast(RHS, GetIntNType(32), false, "expr_castPtr2Int");
        }
        // Note: No pointer will be shown below
        // if as same type
        Type* ty = nullptr;
        if (LHS->getType() == RHS->getType()) {
            ty = LHS->getType();
        }
        else {
            /// TODO: max size of two types
            ty = TypeUtil::raiseType(LHS->getType(), RHS->getType());
            
            // the two type is not compatible
            if (ty == nullptr) {
                errorValue("Cannot apply `{0}' to type `{1}' and `{2}'"_format(op->toSourceLiteral(), "LHS", "RHS"), op);
                addr = ConstantInt::getTrue(GetIntNType(32));
                return;
            }

            // cast if one of them is float type
            if (ty->isFloatTy() || ty->isDoubleTy()) {
                // if coverted type is float type or double type
                if (LHS->getType()->isIntegerTy())
                    LHS = Builder.CreateFPCast(LHS, ty, "expr_fconv");
                if (RHS->getType()->isIntegerTy())
                    RHS = Builder.CreateFPCast(RHS, ty, "expr_fconv");
                
                // if they are not same type
                if (LHS->getType() != ty)
                    LHS = Builder.CreateFPCast(LHS, ty, "expr_fconv");
                if (RHS->getType() != ty)
                    RHS = Builder.CreateFPCast(RHS, ty, "expr_fconv");

                // Calculate float value
                assert(LHS->getType() == RHS->getType() && "LHS and RHS is not with same type");

#define ReportCannotApplyOpOnFloatOperand(op) { errorValue("cannot apply operator '{0}' on float operand"_format(op->toSourceLiteral())); addr = ConstantInt::getIntegerValue(GetIntNType(32), APInt(32, 0, true)); }
                if (op->is('^'))        ReportCannotApplyOpOnFloatOperand(op)
                else if (op->is('|'))   ReportCannotApplyOpOnFloatOperand(op)
                else if (op->is('&'))   ReportCannotApplyOpOnFloatOperand(op)
                else if (op->is(Tag::Equal))    addr = Builder.CreateFCmpUEQ(LHS, RHS, "expr_feq");
                else if (op->is(Tag::NotEqual)) addr = Builder.CreateFCmpUNE(LHS, RHS, "expr_ne");
                else if (op->is('<'))   Builder.CreateFCmpULT(LHS, RHS, "expr_flt");
                else if (op->is('>'))   Builder.CreateFCmpUGT(LHS, RHS, "expr_fgt");
                else if (op->is(Tag::GreaterThanEqual))   addr = Builder.CreateFCmpULE(LHS, RHS, "expr_fle");
                else if (op->is(Tag::LessThanEqual))      addr = Builder.CreateFCmpUGE(LHS, RHS, "expr_fge");
                else if (op->is(Tag::LeftShift))   ReportCannotApplyOpOnFloatOperand(op)
                else if (op->is(Tag::RightShift))  ReportCannotApplyOpOnFloatOperand(op)
                else if (op->is('+'))   addr = Builder.CreateFAdd(LHS, RHS, "expr_add");
                else if (op->is('-'))   addr = Builder.CreateFSub(LHS, RHS, "expr_sub");
                else if (op->is('*'))   addr = Builder.CreateFMul(LHS, RHS, "expr_mul");
                else if (op->is('/'))   addr = Builder.CreateFDiv(LHS, RHS, "expr_udiv");
                else if (op->is('%'))   ReportCannotApplyOpOnFloatOperand(op)
                else assert(false && "invalid op token");
#undef ReportCannotApplyOpOnFloatOperand
                return;
            }
            // else both are integers
            else if (ty->isIntegerTy()) {
                IntegerType* LHS_Ty = static_cast<IntegerType*>(LHS->getType()), *RHS_Ty = static_cast<IntegerType*>(RHS->getType());
                /// check if they are both unsigned
                bool isUnsigned = LHS_Ty->getSignBit() && RHS_Ty->getSignBit();
                if (!isUnsigned) {
                    addr = Builder.CreateSExtOrBitCast(addr, ty, "arith_sext");
                }
                else {
                    addr = Builder.CreateZExtOrBitCast(addr, ty, "arith_zext");
                }

                // Calculate Integer Value
                assert(LHS->getType() == RHS->getType() && "LHS and RHS is not with same type");
                if (op->is('^'))        addr = Builder.CreateXor(LHS, RHS, "bitwise_xor");
                else if (op->is('|'))   addr = Builder.CreateOr(LHS, RHS, "bitwise_or");
                else if (op->is('&'))   addr = Builder.CreateAnd(LHS, RHS, "bitwise_and");
                else if (op->is(Tag::Equal))    addr = Builder.CreateICmpEQ(LHS, RHS, "expr_eq");
                else if (op->is(Tag::NotEqual)) addr = Builder.CreateICmpNE(LHS, RHS, "expr_ne");
                else if (op->is('<'))   addr = isUnsigned ? Builder.CreateICmpULT(LHS, RHS, "expr_ult") : Builder.CreateICmpSLT(LHS, RHS, "expr_slt");
                else if (op->is('>'))   addr = isUnsigned ? Builder.CreateICmpUGT(LHS, RHS, "expr_ugt") : Builder.CreateICmpSGT(LHS, RHS, "expr_sgt");
                else if (op->is(Tag::GreaterThanEqual))   addr = isUnsigned ? Builder.CreateICmpULE(LHS, RHS, "expr_ule") : Builder.CreateICmpSLE(LHS, RHS, "expr_sle");
                else if (op->is(Tag::LessThanEqual))      addr = isUnsigned ? Builder.CreateICmpUGE(LHS, RHS, "expr_uge") : Builder.CreateICmpSGE(LHS, RHS, "expr_sge");
                else if (op->is(Tag::LeftShift))   addr = Builder.CreateShl(LHS, RHS, "expr_shl");
                else if (op->is(Tag::RightShift))  addr = isUnsigned ? Builder.CreateLShr(LHS, RHS, "shr", false) : Builder.CreateAShr(LHS, RHS, "shr", false);
                else if (op->is('+'))   addr = Builder.CreateAdd(LHS, RHS, "expr_add");
                else if (op->is('-'))   addr = Builder.CreateSub(LHS, RHS, "expr_sub");
                else if (op->is('*'))   addr = Builder.CreateMul(LHS, RHS, "expr_mul");
                else if (op->is('/'))   addr = isUnsigned ? Builder.CreateUDiv(LHS, RHS, "expr_udiv") : Builder.CreateSDiv(LHS, RHS, "expr_sdiv");
                else if (op->is('%'))   addr = isUnsigned ? Builder.CreateURem(LHS, RHS, "expr_rem") : Builder.CreateSRem(LHS, RHS, "expr_rem");
                else assert(false && "invalid op token");
                
                return;
            }
        }
        
        
        
    }

    
}