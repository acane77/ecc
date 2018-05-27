#ifndef _MIYUKI_CODEGEN_UTILS_H
#define _MIYUKI_CODEGEN_UTILS_H

#include "ast/env.h"

namespace Miyuki::AST {
	
	class CodeGenUtils {
	public:
		static bool CreateTypeConversion(Value*& t1, Value*& t2);
	};

}

#endif