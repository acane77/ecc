#include "ast/expression.h"
#include "parse/ifparser.h"
#include "ast/value.h"
#include "ast/env.h"
#include "ast/util.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/TypeName.h"
#include "llvm/ADT/APInt.h"

namespace Miyuki::AST {

    using namespace std;

	//// *** static *** ////
	bool askForLValue = false;


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

    bool ConditionalExpression::isConstantExpression() {
        return IsCalculated();
    }

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

    Value* errorvalue(const string& msg, TokenPtr errTok, bool throwError = false) {
        Parse::IParser::instance->diagError(string(msg), errTok);
        if (throwError) throw msg;
        return nullptr;
    }

#define TRY_GEN(x, ...) if(x) x->gen(__VA_ARGS__)
#define ReturnAddressIfCalculated() \
    if (IsCalculated()) {\
        rvalue(ValueFactory::build(getCalculatedToken()));\
        return;\
    }
#define SetExpressLevel(next, sibling) ExpressionPtr nextLevel = next, siblingLevel = sibling
#define InheritSymbolTypeFrom(x) setSymbolType(x->getSymbolType());

#define REPORT_ERROR_V(msg, tok) { \
        errorvalue(msg, tok);\
        rvalue(ZeroValue);\
        return; \
    }
#define LoadIfIsPointer(v) {\
    if (v->getType()->isPointerTy())\
        v = Builder.CreateLoad(v, "arith_"#v); \
    }
#define RequireLValue(expr) { if (!expr->isLValue)  REPORT_ERROR_V("operand of this operator requires lvalue", op) }
#define RequireNonConst(expr)
#define AddressOf(x, y)
#define SetOperatorToken(x) TokenPtr op = x;

    void CommaExpression::gen() {
        ReturnAddressIfCalculated();
        SetExpressLevel(assignExp, commaExp);
        // Code Generation
        TRY_GEN(siblingLevel);
        TRY_GEN(nextLevel);
        // set symbol type & addr
        if (siblingLevel) {
            setAddr(siblingLevel->getAddr(), siblingLevel->isLValue);
            InheritSymbolTypeFrom(siblingLevel);
        }
        else {
            setAddr(nextLevel->getAddr(), nextLevel->isLValue);
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

            Builder.CreateCondBr(logicalOrExp->getAddr(), BB_CondExpr_lhs, BB_CondExpr_rhs);
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
            setAddr(logicalOrExp->getAddr(), logicalOrExp->isLValue);
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

			// Convert to int

            Value * toBool = Builder.CreateICmpNE(logicalAndExp->getAddr(), ZeroValueN(1), "toBool");
            Builder.CreateCondBr(toBool, BB_LOR_end, BB_LOR_rhs);
            
            // Right-hand Statement
            SetInsertBlock(BB_LOR_rhs);
            logicalOrExp->gen();
            Value* toBool1 = Builder.CreateICmpNE(logicalOrExp->getAddr(), ZeroValueN(1), "toBool");
            Builder.CreateBr(BB_LOR_end);

            // End
            SetInsertBlock(BB_LOR_end);
            PHINode* phi = Builder.CreatePHI(Type::getInt1Ty(getGlobalContext()), 2, "");
            phi->addIncoming(ConstantInt::getTrue(Type::getInt1Ty(getGlobalContext())), BB_entry);
            phi->addIncoming(toBool1, BB_LOR_rhs);
            
			rvalue(phi);
            //rvalue(Builder.CreateZExt(phi, GetIntNType(32), "landExt"));
        }
        else {
			logicalAndExp->gen();
            setAddr(logicalAndExp->getAddr(), logicalAndExp->isLValue);
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
			logicalAndExpression->gen();

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
            Value* LHS = exclusiveOrExpression->getAddr(), *RHS = logicalAndExpression->getAddr();
            LoadIfIsPointer(LHS)
            LoadIfIsPointer(RHS)

            Value * toBool = Builder.CreateICmpNE(LHS, ZeroValueN(1), "toBool");
            
            Builder.CreateCondBr(toBool, BB_LAND_rhs, BB_LAND_end);

            // Right-hand Statement
            SetInsertBlock(BB_LAND_rhs);
            logicalAndExpression->gen();
            Value* toBool1 = Builder.CreateICmpNE(RHS, ZeroValueN(1), "toBool");
            Builder.CreateBr(BB_LAND_end);

            // End
            SetInsertBlock(BB_LAND_end);
            PHINode* phi = Builder.CreatePHI(Type::getInt1Ty(getGlobalContext()), 2, "");
            phi->addIncoming(ConstantInt::getFalse(Type::getInt1Ty(getGlobalContext())), BB_entry);
            phi->addIncoming(toBool1, BB_LAND_rhs);

			rvalue(phi);
            //rvalue(Builder.CreateZExt(phi, GetIntNType(32), "landExt"));
        }
        else {
			exclusiveOrExpression->gen();
            setAddr(exclusiveOrExpression->getAddr(), exclusiveOrExpression->isLValue);
        }
    }

    void Arithmetic::gen() {
        ReturnAddressIfCalculated();
        expr1->gen();
        if (expr2 == nullptr) {
            setAddr(expr1->getAddr(), expr1->isLValue);
            return;
        }
        expr2->gen();

        // ** GET LHS and RHS address
        Value *LHS = expr1->getAddr(), *RHS = expr2->getAddr();
        // ** load value of the rwo operands
        //  because LHS and RHS are all pointer pointed to their allocated location
        // so loading first is required (if they are pointer)
		LoadIfIsPointer(LHS)
		LoadIfIsPointer(RHS)

		// Code Below ensures they are with same type
		//if (RHS->getType() != LHS->getType())
		//	if (!CodeGenUtils::CreateTypeConversion(LHS, RHS))
		//		REPORT_ERROR_V("type not castable", op);
		//Type* ty = LHS->getType();
		
        // -- both operands are pointers to qualified or unqualified versions of compatible complete object types
        if (LHS->getType()->isPointerTy() && RHS->getType()->isPointerTy()) {
            // if the two types are not compatible
            // OR is not sub
            if (LHS->getType() != RHS->getType() || op->isNot('-')) 
                REPORT_ERROR_V("Cannot apply `{0}' to type `{1}' and `{2}'"_format(op->toSourceLiteral(), "LHS", "RHS"), op)

            LHS = Builder.CreatePointerBitCastOrAddrSpaceCast(LHS, GetIntNType(32), "expr_castPtr2Int");
            RHS = Builder.CreatePointerBitCastOrAddrSpaceCast(RHS, GetIntNType(32), "expr_castPtr2Int");
            Value * sub = Builder.CreateSub(LHS, RHS, "ptr_sub");
            Value * count = Builder.CreateUDiv(sub, ConstantInt::getIntegerValue(GetIntNType(32), APInt(32, 4)), "get_count");
            rvalue(count);
            return;
        }
        // -- the left operand is a pointer to a complete object type and the right operand has integer type.
        else if (LHS->getType()->isPointerTy()) {
            if (!RHS->getType()->isIntegerTy()) REPORT_ERROR_V("Cannot apply `{0}' to pointer type and non-integer"_format(op->toSourceLiteral()), op)
            GetElementPtrInst* ptrAddSub = nullptr;
            if (op->is('+')) {
                /// TODO:  ** The code that may lead to the leakage of the internal training
                vector<Value*>* v = new vector<Value*>();
                v->push_back(RHS);
                ptrAddSub = GetElementPtrInst::CreateInBounds(LHS, ArrayRef<Value*>(*v), "ptr_add", getCurrentBasicBlock());
            }
            else if (op->is('-')) {
                /// TODO:  ** The code that may lead to the leakage of the internal training
                vector<Value*>* v = new vector<Value*>();
                v->push_back(Builder.CreateSub(ZeroValue, RHS, "neg_rhs"));
                ptrAddSub = GetElementPtrInst::CreateInBounds(LHS, ArrayRef<Value*>(*v), "ptr_sub", getCurrentBasicBlock());
            }
            else REPORT_ERROR_V("Cannot apply `{0}' to pointer type and integer"_format(op->toSourceLiteral()), op);

            lvalue(ptrAddSub);
            return;
        }
        // -- Failure situation: left is arithemetic, and right is Pointer
        else if (RHS->getType()->isPointerTy()) {
            if (!RHS->getType()->isIntegerTy()) REPORT_ERROR_V("Cannot apply `{0}' to non-pointer type and pointer"_format(op->toSourceLiteral()), op)
        }
        // -- Failure situation: one of them has struct type
        else if (RHS->getType()->isStructTy() || LHS->getType()->isStructTy())
            REPORT_ERROR_V("cannot apply arithmetic operations on struct type", op);
        // -- both operands have arithmetic type;
        // Note: No pointer will be shown below
        // if as same type
        Type* ty = nullptr;
        /// raised types
        ty = TypeUtil::raiseType(LHS->getType(), RHS->getType());

        // the two types are not compatible
        if (ty == nullptr) REPORT_ERROR_V("Cannot apply `{0}' to type `{1}' and `{2}'"_format(op->toSourceLiteral(), "LHS", "RHS"), op);
		
        // cast if one of them is float type, and calculate as float
        if (ty->isFloatTy() || ty->isDoubleTy()) {
			
            // if coverted type is float type or double type
			if (LHS->getType()->isIntegerTy()) {
				bool isUnsigned = dyn_cast<IntegerType>(LHS->getType())->getSignBit();
				if (isUnsigned)
					LHS = Builder.CreateSIToFP(LHS, ty, "expr_sitofp");
				else
					LHS = Builder.CreateUIToFP(LHS, ty, "expr_uitofp");
			}
			if (RHS->getType()->isIntegerTy()) {
				bool isUnsigned = dyn_cast<IntegerType>(RHS->getType())->getSignBit();
				if (isUnsigned)
					RHS = Builder.CreateSIToFP(RHS, ty, "expr_sitofp");
				else
					RHS = Builder.CreateUIToFP(RHS, ty, "expr_uitofp");
			}
            // if they are not same type
            //if (LHS->getType() != ty)
            //    LHS = Builder.CreateFPCast(LHS, ty, "expr_fconv");
            //if (RHS->getType() != ty)
            //   RHS = Builder.CreateFPCast(RHS, ty, "expr_fconv");
			if (LHS->getType() != RHS->getType()) {
				if (RHS->getType()->isFloatTy()) {
					RHS = Builder.CreateFPExt(RHS, LHS->getType(), "fp.ext");
				}
				else if (LHS->getType()->isFloatTy()) {
					LHS = Builder.CreateFPExt(LHS, RHS->getType(), "fp.ext");
				}
			}
				
            // Calculate float value
            assert(LHS->getType() == RHS->getType() && "LHS and RHS is not with same type");

#define ReportCannotApplyOpOnFloatOperand(op) { errorvalue("cannot apply operator '{0}' on float operand"_format(op->toSourceLiteral()), op); rvalue(ConstantInt::getIntegerValue(GetIntNType(32), APInt(32, 0, true))); }
            if (op->is('^'))        ReportCannotApplyOpOnFloatOperand(op)
            else if (op->is('|'))   ReportCannotApplyOpOnFloatOperand(op)
            else if (op->is('&'))   ReportCannotApplyOpOnFloatOperand(op)
            else if (op->is(Tag::Equal))    rvalue(Builder.CreateFCmpUEQ(LHS, RHS, "expr_feq"));
            else if (op->is(Tag::NotEqual)) rvalue(Builder.CreateFCmpUNE(LHS, RHS, "expr_ne"));
            else if (op->is('<'))   rvalue(Builder.CreateFCmpULT(LHS, RHS, "expr_flt"));
            else if (op->is('>'))   rvalue(Builder.CreateFCmpUGT(LHS, RHS, "expr_fgt"));
            else if (op->is(Tag::GreaterThanEqual))   rvalue(Builder.CreateFCmpULE(LHS, RHS, "expr_fle"));
            else if (op->is(Tag::LessThanEqual))      rvalue(Builder.CreateFCmpUGE(LHS, RHS, "expr_fge"));
            else if (op->is(Tag::LeftShift))   ReportCannotApplyOpOnFloatOperand(op)
            else if (op->is(Tag::RightShift))  ReportCannotApplyOpOnFloatOperand(op)
            else if (op->is('+'))   rvalue(Builder.CreateFAdd(LHS, RHS, "expr_add"));
            else if (op->is('-'))   rvalue(Builder.CreateFSub(LHS, RHS, "expr_sub"));
            else if (op->is('*'))   rvalue(Builder.CreateFMul(LHS, RHS, "expr_mul"));
            else if (op->is('/'))   rvalue(Builder.CreateFDiv(LHS, RHS, "expr_udiv"));
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
            // covert to same type
            if (LHS_Ty != ty)  LHS = (isUnsigned ? Builder.CreateZExt(LHS, ty, "conv") : Builder.CreateSExt(LHS, ty, "conv"));
            else if (RHS_Ty != ty)  RHS = (isUnsigned ? Builder.CreateZExt(RHS, ty, "conv") : Builder.CreateSExt(RHS, ty, "conv"));
			
            // Calculate Integer Value
            assert(LHS->getType() == RHS->getType() && "LHS and RHS is not with same type");
            if (op->is('^'))        rvalue(Builder.CreateXor(LHS, RHS, "bitwise_xor"));
            else if (op->is('|'))   rvalue(Builder.CreateOr(LHS, RHS, "bitwise_or"));
            else if (op->is('&'))   rvalue(Builder.CreateAnd(LHS, RHS, "bitwise_and"));
            else if (op->is(Tag::Equal))    rvalue(Builder.CreateICmpEQ(LHS, RHS, "expr_eq"));
            else if (op->is(Tag::NotEqual)) rvalue(Builder.CreateICmpNE(LHS, RHS, "expr_ne"));
            else if (op->is('<'))   rvalue(isUnsigned ? Builder.CreateICmpULT(LHS, RHS, "expr_ult") : Builder.CreateICmpSLT(LHS, RHS, "expr_slt"));
            else if (op->is('>'))   rvalue(isUnsigned ? Builder.CreateICmpUGT(LHS, RHS, "expr_ugt") : Builder.CreateICmpSGT(LHS, RHS, "expr_sgt"));
            else if (op->is(Tag::GreaterThanEqual))   rvalue(isUnsigned ? Builder.CreateICmpULE(LHS, RHS, "expr_ule") : Builder.CreateICmpSLE(LHS, RHS, "expr_sle"));
            else if (op->is(Tag::LessThanEqual))      rvalue(isUnsigned ? Builder.CreateICmpUGE(LHS, RHS, "expr_uge") : Builder.CreateICmpSGE(LHS, RHS, "expr_sge"));
            else if (op->is(Tag::LeftShift))   rvalue(Builder.CreateShl(LHS, RHS, "expr_shl"));
            else if (op->is(Tag::RightShift))  rvalue(isUnsigned ? Builder.CreateLShr(LHS, RHS, "shr", false) : Builder.CreateAShr(LHS, RHS, "shr", false));
            else if (op->is('+'))   rvalue(Builder.CreateAdd(LHS, RHS, "expr_add"));
            else if (op->is('-'))   rvalue(Builder.CreateSub(LHS, RHS, "expr_sub"));
            else if (op->is('*'))   rvalue(Builder.CreateMul(LHS, RHS, "expr_mul"));
            else if (op->is('/'))   rvalue(isUnsigned ? Builder.CreateUDiv(LHS, RHS, "expr_udiv") : Builder.CreateSDiv(LHS, RHS, "expr_sdiv"));
            else if (op->is('%'))   rvalue(isUnsigned ? Builder.CreateURem(LHS, RHS, "expr_rem") : Builder.CreateSRem(LHS, RHS, "expr_rem"));
            else assert(false && "invalid op token");

            return;
        }
        
    }

    void Unary::gen() {
        // NOTE: All ID types are saved by their addresses, so the direct return of addresses is taken.
        /// WARNING: FloatType is NOT ConstantFP, SSA[!]
        RETURN_IF_CALCULATED();

		askForLValue = true;
        expr->gen();
		askForLValue = false;

        Value * S = expr->getAddr(), *Ptr = expr->getAddr();
        // S is the address 
        if (op->is('&')) {
            RequireLValue(expr)
            lvalue(S);
            return;
        }
        // Get the actual value of S
        LoadIfIsPointer(S)
        // if is pointer
        if (S->getType()->isPointerTy()) {
            Type * ty = S->getType();
            /// TODO:  ** The code that may lead to the leakage of the internal training
            vector<Value*>* v = new vector<Value*>();

            if (op->is(Tag::Increase)) {
                RequireLValue(expr)  RequireNonConst(expr)
                v->push_back(ConstantInt::getIntegerValue(GetIntNType(32), APInt(32, 1, true)));
            }
            else if (op->is(Tag::Decrease)) {
                RequireLValue(expr)  RequireNonConst(expr)
                v->push_back(ConstantInt::getIntegerValue(GetIntNType(32), APInt(32, -1, true)));
            }
            else if (op->is('!')) {
                S = Builder.CreateICmpNE(S, ConstantPointerNull::get(static_cast<PointerType*>(S->getType())), "toBool");
                S = Builder.CreateXor(S, ConstantInt::getTrue(GetIntNType(32)), "ptr_lnot");
                S = Builder.CreateZExt(S, GetIntNType(32), "ptr_zext");
                rvalue(S);
                return;
            }
            else if (op->is('*')) {
                lvalue(S);
                return;
            }
            else REPORT_ERROR_V("cannot apply '{0}' on function designator"_format(op->toSourceLiteral()), op)
            
            Value * V = GetElementPtrInst::CreateInBounds(S, ArrayRef<Value*>(*v), "ptr_incdec", getCurrentBasicBlock());
			Builder.CreateStore(V, Ptr);
			lvalue(V);
            return;
        }
        // is function designator
        else if (S->getType()->isFunctionTy()) {
            if (op->is('*')) {
                lvalue(S);
            }
            else if (op->is(Tag::Sizeof)) {
                assert(false && "how will code run here? run TypeInfoExpression::gen() instead");
                rvalue(ConstantInt::get(GetIntNType(32), 4, false));
            }
            else REPORT_ERROR_V("cannot apply '{0}' on function designator"_format(op->toSourceLiteral()), op)
            return;
        }
        // else if is float
		// Note: for ++ and --, result is its address, not de/increased value
        else if (Type* ty = S->getType(); ty->isFloatTy() || ty->isDoubleTy()) {
            if (op->is('+')) rvalue(S);
            else if (op->is('-')) rvalue(Builder.CreateFSub(ConstantFP::get(ty, 0), S, "expr_neg_f"));
            else if (op->is(Tag::Increase)) {
                RequireLValue(expr)  RequireNonConst(expr)
                S = Builder.CreateAdd(S, ConstantFP::get(ty, 1));
				Builder.CreateStore(S, Ptr);
				lvalue(S); // address as lvalue
            }
            else if (op->is(Tag::Decrease)) {
                RequireLValue(expr)  RequireNonConst(expr)
                S = Builder.CreateSub(S, ConstantFP::get(ty, 1));
				Builder.CreateStore(S, Ptr);
				lvalue(S); // address as lvalue
            }
            else assert(false && "invalid operator");
            return;
        }
        // else if integer
        else if (Type* ty = S->getType(); ty->isIntegerTy()) {
            bool isSigned = !static_cast<IntegerType*>(ty)->getSignBit();
            if (op->is('+')) rvalue(S);
            else if (op->is('-')) rvalue(Builder.CreateSub(ZeroValue, S, "expr_neg"));
            else if (op->is('!')) {
                S = Builder.CreateICmpEQ(S, ZeroValue, "toBool");
                rvalue(isSigned ? Builder.CreateSExt(S, ty, "sext") : Builder.CreateZExt(S, ty, "zext"));
            }
            else if (op->is('~')) rvalue(Builder.CreateXor(S, ConstantInt::get(ty, -1)));
            else if (op->is('&')) {
				assert(!"????");
                RequireLValue(expr)
                rvalue(Ptr);
            }
			// For a ++ and --
			// load them and inc/dec then set address as lvalue
            else if (op->is(Tag::Increase)) { 
                RequireLValue(expr)  RequireNonConst(expr)
                Value* inc = Builder.CreateAdd(S, ConstantInt::get(ty, 1, true), "inc");
                Builder.CreateStore(inc, Ptr);
				lvalue(inc);
            }
            else if (op->is(Tag::Decrease)) { 
                RequireLValue(expr)  RequireNonConst(expr)
                Value* newVal = Builder.CreateSub(S, ConstantInt::get(ty, 1, true), "dec");
                Builder.CreateStore(newVal, Ptr);
				lvalue(newVal);
            }
            else assert(false && "invalid operator");
            return;
        }
        // else is struct type
        else if (ty->isStructTy()) {
            if (op->is('&')) rvalue(S);
            else if (op->is('*')) REPORT_ERROR_V("Cannot apply dereference on non-pointer type", op)
            else REPORT_ERROR_V("cannot apply arithemetic operator on struct type", op);
        }
        else REPORT_ERROR_V("cannot apply this operator with the operands.", op);
    }

    void LogicalNot::gen() {
        Unary::gen();
    }

    void TypeInfoExpression::gen() {
        Type* ty = typeName->getType()->type;
        if (op->is(Tag::Sizeof)) {
            if (ty->isPointerTy()) {
                rvalue(ConstantInt::get(GetIntNType(32), PointerSize, false));
                return;
            }
            size_t size = ty->getIntegerBitWidth();
            rvalue(ConstantInt::get(GetIntNType(32), size / 8, false));
        }
        else if (op->is(Tag::Alignof)) {
            /// TODO: IMPLEMENT THIS.
            // Note: NOT SUPPORTED UTILL NOW
            rvalue(ConstantInt::get(GetIntNType(32), PointerSize, false));
        }
        else assert("neither sizeof nor alignof");
    }

    void PostfixExpression::gen() {
        // Note: only postfix ++ -- generate here
        assert((op->is(Tag::Increase) || op->is(Tag::Decrease)) && "invalid operators");
		PostfixExpressionPtr expr = postfixExp;
		askForLValue = true;
        postfixExp->gen();
		askForLValue = false;
        Value * S = postfixExp->getAddr();
        // load actual value of S
        LoadIfIsPointer(S)
        
        Type* ty = S->getType();
        if (ty->isStructTy()) REPORT_ERROR_V("cannot apply this operator on struct type", op)
        else if (ty->isIntegerTy()) {
            if (op->is(Tag::Increase)) {
                RequireLValue(expr)  RequireNonConst(expr)
                Value* newVal = Builder.CreateAdd(S, ConstantInt::get(ty, 1, true), "new_val");
                Builder.CreateStore(newVal, postfixExp->getAddr());
                rvalue(S);
            }
            else if (op->is(Tag::Decrease)) {
                RequireLValue(expr)  RequireNonConst(expr)
                Value* newVal = Builder.CreateSub(S, ConstantInt::get(ty, 1, true), "new_val");
                Builder.CreateStore(newVal, postfixExp->getAddr());
                rvalue(S);
            }
        }
        else if (ty->isFloatTy()) {
            if (op->is(Tag::Increase)) {
                RequireLValue(expr)  RequireNonConst(expr)
                Value* newVal = Builder.CreateFAdd(S, ConstantFP::get(ty, 1.0), "new_val");
                Builder.CreateStore(newVal, postfixExp->getAddr());
                rvalue(S);
            }
            else if (op->is(Tag::Decrease)) {
                RequireLValue(expr)  RequireNonConst(expr)
                Value* newVal = Builder.CreateFSub(S, ConstantFP::get(ty, 1.0), "new_val");
                Builder.CreateStore(newVal, postfixExp->getAddr());
                rvalue(S);
            }
        }
        else if (ty->isPointerTy()) {
            vector<Value*>* v = new vector<Value*>();

            if (op->is(Tag::Increase)) {
                RequireLValue(expr)  RequireNonConst(expr)
                v->push_back(ConstantInt::getIntegerValue(GetIntNType(32), APInt(32, 1, true)));
            }
            else if (op->is(Tag::Decrease)) {
                RequireLValue(expr)  RequireNonConst(expr)
                v->push_back(ConstantInt::getIntegerValue(GetIntNType(32), APInt(32, -1, true)));
            }
            Value* newVal = GetElementPtrInst::CreateInBounds(S, ArrayRef<Value*>(*v), "ptr_incdec", getCurrentBasicBlock());
            Builder.CreateStore(newVal, postfixExp->getAddr());
            rvalue(S);
        }
    }

    void StructAccess::gen() {
        // struct access provide two method of access
        /// TODO: Implement it
        assert(!"Unimplemented");
    }

    void AnonymousArray::gen() {
        REPORT_ERROR_V("Anonymous array is not supported yet.", getErrorToken());
    }

#define ErrorIfLHSIsPoiner() {\
    if (cannotDoThisOperation) REPORT_ERROR_V("cannot do this operationn on LHS & RHS", getErrorToken());\
}
    void AssignmentExpression::gen() {

		askForLValue = assignExp && true; // If is assign exp
        condExp->gen();
		askForLValue = false;

        if (!assignExp) {
            setAddr(condExp->getAddr(), condExp->isLValue);
            return;
        }
        assignExp->gen();
        Value * LHS = condExp->getAddr(), *RHS = assignExp->getAddr();
		Value* LHSPtr = condExp->getAddr();
        Value * V = nullptr; // Value of calculation result
        
        SetOperatorToken(assignOp)
        LoadIfIsPointer(LHS)
        LoadIfIsPointer(RHS)
        RequireLValue(condExp)
        RequireNonConst(condExp)

        if (PointerType* ty = dyn_cast<PointerType>(LHS->getType())) {
            // try convert type of rvalue
            Type * rt = RHS->getType();
            // only for pointer - poiner,
            bool cannotDoThisOperation = false;
            
            // convert type
            if (ty == rt); //pass, no conversion required
            else if (rt->isIntegerTy()) {
                RHS = Builder.CreateIntCast(RHS, GetIntNType(32), true, "rhs.castToInt");
            }
            else if (rt->isFloatingPointTy())  REPORT_ERROR_V("pointer cannot assign floating-point", getErrorToken())
            else if (rt->isPointerTy()) {
                // only allowed for minus
                RHS = Builder.CreateIntCast(RHS, GetIntNType(32), true, "sub.ptr.rhs.castToInt");
                cannotDoThisOperation = true;
            }
            else REPORT_ERROR_V("pointer cannot assign object type", getErrorToken())

            if (op->is('=')) 
                V = RHS;
            else if (op->is(Tag::AddAssign)) {
                ErrorIfLHSIsPoiner();
                vector<Value*>* v = new vector<Value*>();
                v->push_back(RHS);
                V = GetElementPtrInst::Create(ty, condExp->getAddr(), ArrayRef<Value*>(*v), "ptr.add", getCurrentBasicBlock());
            }
            else if (op->is(Tag::SubAssign)) {
                // for pointer minus pointer, 
                // DIFFERENT FROM OTHERSITUATION:
                //  result of this operation int32 so convert to pointer type, using int to ptr
                // TODO:  Generate a warning for ptr-ptr
                
                //convert LHS to integer
                LHS = Builder.CreateIntCast(LHS, GetIntNType(32), true, "sub.ptr.lhs.castToInt");
                LHS = Builder.CreateSub(LHS, RHS, "ptr.sub");
                V = Builder.CreateIntToPtr(LHS, ty, "int2ptr");
            }
            else REPORT_ERROR_V("cannot perform this operation on the two operands", getErrorToken());
        }
        else if (IntegerType* ty = dyn_cast<IntegerType>(LHS->getType())) {
            // try convert type of rvalue
            Type * rt = RHS->getType();

            // convert type
            if (ty == rt); //pass, no conversion required
            else if (rt->isIntegerTy()) {
                RHS = Builder.CreateIntCast(RHS, ty, !ty->getSignBit(), "castToInt");
            }
            else if (rt->isFloatingPointTy()) {
                RHS = Builder.CreateIntCast(RHS, ty, !ty->getSignBit(), "castToInt");
            }
            else if (rt->isPointerTy()) REPORT_ERROR_V("pointer cannot assign to integer type", getErrorToken())
            else REPORT_ERROR_V("integer cannot assign object type", getErrorToken())

            if (op->is('='))
                V = RHS;
            else if (op->is(Tag::AddAssign)) {
                V = Builder.CreateAdd(LHS, RHS, "int.add");
            }
            else if (op->is(Tag::SubAssign)) {
                V = Builder.CreateSub(LHS, RHS, "int.sub");
            }
            else if (op->is(Tag::MulpileAssign)) {
                V = Builder.CreateMul(LHS, RHS, "int.mul");
            }
            else if (op->is(Tag::DivideAssign)) {
                V = ty->getSignBit() ? Builder.CreateUDiv(LHS, RHS, "int.udiv") : Builder.CreateSDiv(LHS, RHS, "int.sdiv");
            }
            else if (op->is(Tag::ModAssign)) {
                V = ty->getSignBit() ? Builder.CreateURem(LHS, RHS, "int.urem") : Builder.CreateSRem(LHS, RHS, "int.rem");
            }
            else if (op->is(Tag::LeftShiftAssign)) {
                V = Builder.CreateShl(LHS, RHS, "expr_shl");
            }
            else if (op->is(Tag::RightShiftAssign)) {
                V = ty->getSignBit() ? Builder.CreateLShr(LHS, RHS, "shr", false) : Builder.CreateAShr(LHS, RHS, "shr", false);
            }
            else if (op->is(Tag::BitwiseAndAssign)) {
                V = Builder.CreateAnd(LHS, RHS, "bitwise_and");
            }
            else if (op->is(Tag::BitwiseOrAssign)) {
                V = Builder.CreateOr(LHS, RHS, "bitwise_or");
            }
            else if (op->is(Tag::XorAssign)) {
                V = Builder.CreateXor(LHS, RHS, "bitwise_xor");
            }
            else assert(!"unexpected operator");
        }
        else if (Type * ty = LHS->getType(); ty->isFloatingPointTy()) {
            // try convert type of rvalue
            Type * rt = RHS->getType();

            // convert type
            if (ty == rt); //pass, no conversion required
            if (rt->isPointerTy()) REPORT_ERROR_V("cannot assign a pointer to float", getErrorToken())
            else if (rt->isIntegerTy()) {
                RHS = static_cast<IntegerType*>(rt)->getSignBit() ? Builder.CreateUIToFP(RHS, ty, "fp.cast") : Builder.CreateUIToFP(RHS, ty, "fp.cast");
            }
            else if (rt->isFloatingPointTy()) {
                RHS = Builder.CreateFPTrunc(RHS, ty, "fp.cast");
            }
            else REPORT_ERROR_V("float cannot assign object type", getErrorToken())

            if (op->is('=')) {
                V = RHS;
            }
            else if (op->is(Tag::AddAssign)) {
                V = Builder.CreateFAdd(LHS, RHS, "int.fadd");
            }
            else if (op->is(Tag::SubAssign)) {
                V = Builder.CreateFSub(LHS, RHS, "int.fsub");
            }
            else if (op->is(Tag::MulpileAssign)) {
                V = Builder.CreateFMul(LHS, RHS, "int.fmul");
            }
            else if (op->is(Tag::DivideAssign)) {
                V = Builder.CreateFDiv(LHS, RHS, "int.fdiv");
            }
            else REPORT_ERROR_V("cannot perform this operation on the two operands", getErrorToken());
        }
        else REPORT_ERROR_V("cannot assign such type", getErrorToken())

        // store in memory
        Builder.CreateStore(V, condExp->getAddr());
        rvalue(V);
    }

	void Miyuki::AST::PrimaryExpression::gen() {
		if (factor) {
			// identifier
			if (WordTokenPtr id = dynamic_pointer_cast<WordToken>(factor)) {
				// Get function from function list
				if (Function* F = TheModule->getFunction(id->name)) {
					if (askForLValue) {
						REPORT_ERROR_V("`{0}' is a function name, which cannot be an lvalue"_format(id->name), factor)
					}
					rvalue(F);
					return;
				}
				// Get from identifier list
				else if (IdentifierPtr ID = getIdentifier(id->name)) {
					// if ask for lvalue, return address
					if (askForLValue) {
						lvalue(ID->getAddr());
						return;
					}
					// else return loaded value
					Value* V = Builder.CreateLoad(ID->getAddr(), "{0}.ld"_format(id->name));
					rvalue(V); return;
				}
				REPORT_ERROR_V("`{0}' is not defined"_format(id->name), factor)
			}
			// constant
			else if (factor->is(Tag::Number | Tag::StringLiteral | Tag::Character)) {
				Value* V = TypeUtil::createConstant(factor);
				rvalue(V); return;
			}
			// ( expression )
			else if (exp) {
				exp->gen();
				setAddr(exp->getAddr(), exp->isLValue);
				return;
			}
			assert(!"invalid primary");
		}
		assert(!"Not implemented yet");
	}

	void Miyuki::AST::CastExpression::gen() {
		if (unaryExpr) {
			unaryExpr->gen();
			setAddr(unaryExpr->getAddr(), unaryExpr->isLValue);
			return;
		}
		
		PackedTypeInformationPtr PP = typeName->getType();
		TypePtr typeCastTo = PP->type;
		castExpr->gen();
		TypePtr typeToCast = castExpr->getAddr()->getType();
		
		// Check compatiblity
		TypePtr raisedTy = nullptr;
		if (!TypeUtil::raiseType(typeCastTo, typeCastTo)) {
			REPORT_ERROR_V("type not compatible", getErrorToken());
		}

		// Convert

		assert(!"Not implemented yet");

	}

	void FunctionCall::gen() {
		postfixExp->gen();
		vector<Value*>* Args = new vector<Value*>();
		if (argExprLst)
			argExprLst->gen(Args);

		Value* FAddr = postfixExp->getAddr();
		Function * F = nullptr;

		auto VerifyFunctionArgs = [&](Function* F)->int {
			unsigned N = F->getType()->getPointerElementType()->getFunctionNumParams();
			
			// if num of para not the same as num of args
			if (N > Args->size() || (N < Args->size() && !F->getType()->getPointerElementType()->isFunctionVarArg()))
				return -2;
			// check every params
			for (int i = 0; i < N; i++) {
				if (!TypeUtil::raiseType(F->getType()->getPointerElementType()->getFunctionParamType(i), (*Args)[i]->getType()))
					return i;

				//try convert
				Type* DstTy = F->getType()->getPointerElementType()->getFunctionParamType(i),
					*SrcTy = (*Args)[i]->getType();

				if (DstTy == SrcTy)
					continue;

				Value* RHS = (*Args)[i];

				if (PointerType* ty = dyn_cast<PointerType>(DstTy)) {
					// try convert type of rvalue
					Type * rt = SrcTy;

					// convert type
					if (ty == rt); //pass, no conversion required
					else if (rt->isIntegerTy()) {
						RHS = Builder.CreateIntCast(RHS, GetIntNType(32), true, "rhs.castToInt");
					}
					else if (rt->isFloatingPointTy()) {
						errorvalue("pointer cannot assign floating-point", getErrorToken());
						return i;
					}
					else if (rt->isPointerTy()) {
						// only allowed for minus
						RHS = Builder.CreateIntCast(RHS, GetIntNType(32), true, "sub.ptr.rhs.castToInt");
					}
					else {
						errorvalue("pointer cannot assign object type", getErrorToken());
						return i;
					}
				}
				else if (IntegerType* ty = dyn_cast<IntegerType>(DstTy)) {
					// try convert type of rvalue
					Type * rt = SrcTy;

					// convert type
					if (ty == rt); //pass, no conversion required
					else if (rt->isIntegerTy()) {
						RHS = Builder.CreateIntCast(RHS, ty, !ty->getSignBit(), "castToInt");
					}
					else if (rt->isFloatingPointTy()) {
						RHS = Builder.CreateIntCast(RHS, ty, !ty->getSignBit(), "castToInt");
					}
					else if (rt->isPointerTy()) {
						errorvalue("pointer cannot assign to integer type", getErrorToken());
						return i;
					}
					else {
						errorvalue("integer cannot assign object type", getErrorToken());
						return i;
					}
				}
				else if (Type * ty = DstTy; ty->isFloatingPointTy()) {
					// try convert type of rvalue
					Type * rt = SrcTy;

					// convert type
					if (ty == rt); //pass, no conversion required
					if (rt->isPointerTy()) {
						errorvalue("cannot assign a pointer to float", getErrorToken());
						return i;
					}
					else if (rt->isIntegerTy()) {
						RHS = static_cast<IntegerType*>(rt)->getSignBit() ? Builder.CreateUIToFP(RHS, ty, "fp.cast") : Builder.CreateUIToFP(RHS, ty, "fp.cast");
					}
					else if (rt->isFloatingPointTy()) {
						RHS = Builder.CreateFPTrunc(RHS, ty, "fp.cast");
					}
					else {
						errorvalue("float cannot assign object type", getErrorToken());
						return i;
					}
				}
				else {
					errorvalue("cannot convert such type", getErrorToken());
					return i;
				}

				(*Args)[i] = RHS;
			}
			return -1;
		};
		
		/// Find function
		F = dyn_cast<Function>(FAddr);
		if (F = dyn_cast<Function>(FAddr)) {
function:
			int verify = VerifyFunctionArgs(F);
			if (verify == -2) {
				REPORT_ERROR_V("function parameter number does not match", postfixExp->getErrorToken());
			}
			else if (verify != -1) {
				REPORT_ERROR_V("function parameter #{0} does not match and not castable"_format(verify), postfixExp->getErrorToken());
			}
			Value* V = Builder.CreateCall(FAddr, *Args, "call");
			rvalue(V);
			return;
		}

		/// else is function address
		else if (FAddr->getType()->isPointerTy()) {
			FunctionType* FT = dyn_cast<FunctionType>(FAddr->getType()->getPointerElementType());
			if (!FT) {
				REPORT_ERROR_V("postfix-expression is not function type", postfixExp->getErrorToken());
			}
			F = dyn_cast<Function>(Builder.CreateBitCast(FAddr, FT, "ptr.knr.cast"));
			assert(F && "not a function type");
			goto function;
			return;
		}

		REPORT_ERROR_V("postfix-expression is not function type", postfixExp->getErrorToken());
	}

	void ArgumentExpressionList::gen(vector<Value*>* Args) {
		assignExpr->gen();
		Args->push_back(assignExpr->getAddr());
		if (argExprLst)
			argExprLst->gen(Args);
	}

}

/// TODO: In all eval functions, 对于无法求值的情况，如果出现了常量，那么说明尝试对常量进行求值
///  在这种情况下需要报错对左值进行运算
///  添加const的测试

// TODO: 在astbuilder中添加适合报错的errortoken

// 任何时候都记住  addr正如其名  是地址