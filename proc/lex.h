#ifndef _MIYUKI_PROCLEXER_H
#define _MIYUKI_PROCLEXER_H

#include "include.h"
#include "common/flread.h"
#include "proc/token.h"

namespace Miyuki::Proc {
    using namespace Miyuki::Common;
    class Lexer {
        FileReadPtr M_fr;
        int peak;

        char _getCharFromSlash();

    public:
        Lexer(FileReadPtr fr_ptr);

        void readch() { peak = M_fr->nextChar(); }
        void retract() { peak = M_fr->lastChar(); }
        string getLine(int i) { return M_fr->getLine(i); }
        string getCurrentLine() { return M_fr->getLine(); }
        void backToPos(int row, int col) { M_fr->to(row, col); }
        int getColumn() { return M_fr->getColumn(); }
        int getRow() { return M_fr->getRow(); }

        TokenPtr scan();

        ~Lexer();
    };
}

#endif
