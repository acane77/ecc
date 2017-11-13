#include "imlex.h"

namespace Miyuki::Lex {

    TokenPtr IntermediateLexer::getRealToken(TokenPtr tok) {
        index = 0;
        ppLitTok = tok;
        literalString = tok->toSourceLiteral();
        TokenPtr ret = scan();
        ret->copyAdditionalInfo(tok);
        return ret;
    }
}