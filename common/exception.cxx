#include "exception.h"

namespace Miyuki::Common {

    const char * IOException::what() const noexcept {
       return "I/O Exception: no such file or directory.";
    }

    const char * InvalidToken::what() const noexcept {
       return "invalid token found.";
    }

    const char *SyntaxError::what() const noexcept {
        return "syntax error.";
    }

}
