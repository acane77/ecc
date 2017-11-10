#ifndef _MIYUKI_SYMBOLS_H
#define _MIYUKI_SYMBOLS_H

#include "lex/token.h"
#include "common/ptrdef.h"

namespace Miyuki::AST {

    DEFINE_SHARED_PTR(Symbol)

    class IEvaluatable {
    public:
        virtual void eval() = 0;
    };

    class Symbol {

    };

}

#endif