#include <common/flread.h>
#include "token.h"

namespace Miyuki::Proc {
    Miyuki::Common::FileReadPtr Token::flread = nullptr;
    int Token::startColumn = 0;
}
