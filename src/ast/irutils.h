#ifndef _MIYUKI_IR_UTILS_H
#define _MIYUKI_IR_UTILS_H

#include "ast/env.h"

namespace Miyuki::AST {

	extern TokenPtr __irutils_err_token;

#define SetCommonErrorTokenForIRUtils(x) __irutils_err_token = x;

	// Report errpr
	void reportError(const string& msg, TokenPtr errTok, bool throwError = false);

	// Build instructions to convert float to int
	Value* convertToInt(Value* Val, Type* intTy);

	// Build comparasion-to-0 instructions
	// RETURN: nullptr if invalid comparasion
	//         pointer-to-value if ok
	Value* createComparationToZero(Value *L);

}

#endif