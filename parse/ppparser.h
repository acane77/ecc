#ifndef _MIYUKI_PPPARSER_H
#define _MIYUKI_PPPARSER_H

#include "ifparser.h"
#include "lex/pplex.h"

namespace Miyuki::Parse {

    using namespace Lex;

    // This file defined preprocessor parser
    class PreprocessorParser : public IParser {
        PreprocessorLexerPtr M_pplex;

    public:
        explicit PreprocessorParser(const char * path) : M_pplex(make_shared<PreprocessorLexer>()) {
            M_pplex->openFile(path);
            M_lex = M_pplex;
        }

        void parse() final;

    };

}

#endif