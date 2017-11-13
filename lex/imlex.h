#ifndef _MIYUKI_IMLEX_H
#define _MIYUKI_IMLEX_H

// this file defines intermediate lexer

#include "lex/lex.h"

namespace Miyuki::Lex {

    class IntermediateLexer : public Lexer {
    public:
        PPLiteralTokenPtr ppLitTok;
        string literalString;
        int index;

        IntermediateLexer() : Lexer(0) { }

        // Initialize ppLitTok and Index, and then scan using function
        // in parent class.
        TokenPtr getRealToken(PPLiteralTokenPtr tok);

        void readch() override { peak = literalString[index++]; }
        void retract() override { peak = literalString[--index]; }
    };

}

#endif