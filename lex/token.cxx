#include "common/flread.h"
#include "token.h"

namespace Miyuki::Lex {
    Miyuki::Common::FileReadPtr Token::flread = nullptr;
    int Token::startColumn = 0;
}
