#ifndef _MIYUKI_AST_VALUE_H
#define _MIYUKI_AST_VALUE_H

#include "llvm/IR/Value.h"
#include "lex/token.h"

namespace Miyuki::AST {
    
    using namespace Miyuki::Lex;
    using namespace llvm;

    class ValueFactory {
    public:
        static Value* build(TokenPtr tok);
    };

    class ValueFlag {
    public:
        // specify if expression is in conditional expression
        // Note: if in cond expr, generate conditional branch
        //  instead of calculating it.
        static bool isConditionExpression;
    };

}

#endif