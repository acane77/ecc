#ifndef _MIYUKI_EXCEPTION_H
#define _MIYUKI_EXCEPTION_H

#include <exception>
#include <string>

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
        string msg;

        SyntaxError(string& _msg) { msg = std::move(_msg); }
        SyntaxError(string&& _msg) { msg = _msg; }

        const char * what() const noexcept override;
    };

}
#endif
