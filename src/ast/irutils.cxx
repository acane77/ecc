#include "irutils.h"
#include "irutils.h"
#include "ast/irutils.h"
#include "ast/env.h"

namespace Miyuki::AST {
	
	TokenPtr __irutils_err_token = nullptr;

	/// Reprot error
	void reportError(const string& msg, TokenPtr errTok, bool throwError) {
		Parse::IParser::instance->diagError(string(msg), errTok);
		if (throwError) throw msg;
	}

	Value* convertToInt(Value * Val, Type * intTy) {
		Type * T = Val->getType();
		if (T->isFloatingPointTy()) {
			return Builder.CreateIntCast(Val, intTy, true, "conv_to_int");
		}
		else if (T->isPointerTy()) {
			return Builder.CreatePointerBitCastOrAddrSpaceCast(L, intTy, "castPtr2Int");
		}
		else if (T->isIntegerTy*()) {
			return Val;
		}
		reportError("cannot convert to int", __irutils_err_token);
		return ZeroValueN(dyn_cast<IntegerType>(T)->getBitWidth());
	}

	Value* createComparationToZero(Value * L) {
		// if any of L or R is float type then float
		Type * LT = L->getType();
		if (LT->isFloatingPointTy()) {
			L = Builder.CreateFCmpUNE(L, ConstantFP::get(LT, 0), "fcmp_c0");
		}
		else if (LT->isPointerTy()) {
			L = Builder.CreateICmpNE(L, ConstantInt::getNullValue(LT), "icmp_c0");
		}
		else if (IntegerType* I = dyn_cast<IntegerType>(LT);  LT->isIntegerTy()) {
			L = Builder.CreateICmpNE(L, ZeroValueN(I->getBitWidth()), "icmp_c0");
		}
		else {
			reportError("cannot compare to int", __irutils_err_token);
			L = ConstantInt::get(Type::getInt1Ty(), APInt(1, 0));
		}
		return L;
	}

	

}