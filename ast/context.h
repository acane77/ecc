#ifndef _MIYUKI_CONTEXT_H
#define _MIYUKI_CONTEXT_H

#include "ast/context.h"
#include "ast/function.h"
#include "type.h"

namespace Miyuki::AST {

	class Context {
	public:
		typedef std::deque<StructTypePtr> StructNesting;
		StructNesting structNesting;
		FunctionPtr thisFunction;
		
		void getTypeNamePrefix();
	};

}

#endif // _MIYUKI_CONTEXT_H