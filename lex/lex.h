#ifndef _MIYUKI_LEXER_H
#define _MIYUKI_LEXER_H

#include "include.h"
#include "common/flread.h"
#include "lex/token.h"
#include "interfaces.h"

namespace Miyuki::Lex {
    using namespace Miyuki::Common;
    class Lexer {
    protected:
        FileReadPtr M_fr;
        int peak;

        // Get character value with a slash
        char _getCharFromSlash();
        // Skip commemt and spaces, returns no token.
        // Return value: wheather program meets EOF.
        virtual bool eatCommentAndSpaces();
        // Scan punctuators
        virtual TokenPtr scanPunctuators();
        virtual TokenPtr scanKeywordOrIdentifier(string& word);
        virtual TokenPtr scanIdentifierAndStringCharacterLiteral();
        virtual TokenPtr scanStringCharacterLiteral(uint32_t encoding);
        virtual TokenPtr scanIntegerAndFloating();

    public:
        explicit Lexer(FileReadPtr fr_ptr);

        void readch() { peak = M_fr->nextChar(); }
        void retract() { peak = M_fr->lastChar(); }
        string getLine(int i) { return M_fr->getLine(i); }
        string getCurrentLine() { return M_fr->getLine(); }
        void backToPos(int row, int col) { M_fr->to(row, col); }
        int getColumn() { return M_fr->getColumn(); }
        int getRow() { return M_fr->getRow(); }

        virtual TokenPtr scan();

        ~Lexer();
    };
}

#endif
