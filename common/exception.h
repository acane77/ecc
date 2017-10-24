#ifndef _MIYUKI_EXCEPTION_H
#define _MIYUKI_EXCEPTION_H

#include <exception>

using namespace std;

namespace Miyuki::Common {
    class IOException : public exception {
    public:
        const char * what() const noexcept override;
    };

    class InvalidToken : public exception {
    public:
        const char * what() const noexcept override;
    };

    class SyntaxError : public exception {
    public:
        const char * what() const noexcept override;
    };
}

#endif
