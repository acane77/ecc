#ifndef _MIYUKI_IMLEX_H
#define _MIYUKI_IMLEX_H

// this file defines intermediate lexer

#include "lex/lex.h"

namespace Miyuki::Lex {

    DEFINE_SHARED_PTR(IntermediateLexer)

    class IntermediateLexer : public Lexer {
    public:
        TokenPtr ppLitTok;
        string literalString;
        int index;

        IntermediateLexer() : Lexer(0) { }

        // Initialize ppLitTok and Index, and then scan using function
        // in parent class.
        TokenPtr getRealToken(TokenPtr tok);

        void readch() override {
            if ( index == literalString.size() ) { peak = -1; return; }
            peak = literalString[index++];
        }
        void retract() override { peak = literalString[--index]; }
        virtual int getColumn() { return ppLitTok->startCol; }
        virtual int getRow() { return ppLitTok->row; }
    };

}

#endif