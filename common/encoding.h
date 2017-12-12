#ifndef _MIYUKI_ENCODING_H
#define _MIYUKI_ENCODING_H

#include <cstdint>
#include <exception>
#include <string>
#include <cassert>

namespace Miyuki::Common {

    class Encoding {
    private:
        uint32_t encoding;
        const char * encodingString;

    public:
        enum Prefix { U = 0, u, u8, L, ASCII };
        explicit Encoding(uint32_t _encoding) { setEncoding(_encoding); }
        Encoding& operator=(Encoding& enc) { encoding = enc.encoding; encodingString = enc.encodingString; return *this; }
        Encoding& operator=(uint32_t _encoding) { setEncoding(_encoding); return *this; }
        uint32_t getEncoding() { return encoding; }
        const char * getEncodingString() { return encodingString; }
        void setEncoding(uint32_t encoding) {
            Encoding::encoding = encoding;
            if (encoding == U) encodingString = "U";
            else if (encoding == u) encodingString = "u";
            else if (encoding == u8) encodingString = "u8";
            else if (encoding == L) encodingString = "L";
            else if (encoding == ASCII) encodingString = "";
            else assert(false && "invalid encoding");
        };
        static uint32_t getEncodingFromString(std::string& enc) {
            if (enc == "U")  return U;
            else if (enc == "u") return u;
            else if (enc == "u8") return u8;
            else if (enc == "L") return L;
            else assert(false && "invalid encoding");
        }
    };

}

#endif