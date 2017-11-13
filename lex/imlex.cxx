#include "imlex.h"

namespace Miyuki::Lex {

    TokenPtr IntermediateLexer::getRealToken(TokenPtr tok) {
        index = 0;
        ppLitTok = tok;
        literalString = tok->toSourceLiteral();
        return scan();
    }
}