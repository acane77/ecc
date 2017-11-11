#ifndef _MIYUKI_ASTBUILDER_H
#define _MIYUKI_ASTBUILDER_H

#include "parse/ifparser.h"

// this file defines AST Builder, which builds Anstract Syntax Tree
// this class is common use in both preprocessor and parser

namespace Miyuki::AST {

    using namespace Miyuki::Parse;

    class ASTBuilder : public IParser {
    public:
        virtual TokenPtr buildXXX() { assert( false && "do not use this function, just a sample function." ); }
    };

}

#endif