#ifndef _MIYUKI_FUNCTION_H
#define _MIYUKI_FUNCTION_H

#include "common/defines.h"

namespace Miyuki::AST {

	DEFINE_SHARED_PTR(Function);

	class Function {
	public:
		string name;
		FunctionTypePtr type;
	};

}

#endif