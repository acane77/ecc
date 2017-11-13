#include "imlex.h"

namespace Miyuki::Lex {

    TokenPtr IntermediateLexer::getRealToken(TokenPtr tok) {
        index = 0;
        ppLitTok = tok;
        literalString = tok->toSourceLiteral();
        TokenPtr ret = scan();
        ret->startCol = tok->startCol;
        ret->row = tok->row;
        ret->column = tok->column;
        ret->filenam = tok->filenam;
        return ret;
    }
}