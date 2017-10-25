#ifndef _MIYUKI_PROCTOKEN_H
#define _MIYUKI_PROCTOKEN_H

#include "common/flread.h"
#include "include.h"
#include "tags.h"
#include "common/ptrdef.h"
#include <format.h>

using namespace std;
using namespace fmt;

namespace Miyuki::Proc {

    // This file defines tokens (ref ISO/IEC 9899:201x $A.1)

    DEFINE_SHARED_PTR(Token)
    DEFINE_SHARED_PTR(WordToken)
    DEFINE_SHARED_PTR(IntToken)
    DEFINE_SHARED_PTR(FloatToken)
    DEFINE_SHARED_PTR(CharToken)

    // Base type of tokens
    class Token {
    public:
        // information in source code
        int column;
        int row;
        int chrlen;
        const char * filenam;

        // characteristic information
        int tag;

        // use for getting know about more information
        static Common::FileReadPtr flread;
        static int startColumn;

        Token(int _tag) { tag = _tag; if (flread) { column = flread->getColumn(); row = flread->getRow(); filenam = ""; chrlen = column - startColumn; } }
        virtual string toString() {
            if (tag > 31 && tag < 127)  return "Sign: {0}"_format((char)tag);
            return "Token[tag={0}]"_format(tag);
        }
    };

    // Identifier & Keyword & Enumration
    class WordToken : public Token {
    public:
        string name;

        // For Identifier
        WordToken(string _name) :Token(Tag::ID) { name = _name; }

        // For keywords
        WordToken(int _tag, string _name) :Token(_tag) { name = _name; }

        // For Enmeration
        void changeTypeToEnumeration() { tag = Tag::Enumeration; }

        string toString() { return "{0}: {1}"_format(tag == Tag::ID ? "Identifier" : ( tag == Tag::Enumeration ? "Enumeration" : "Keyword" ),name); }
    };

    // Integer Constant
    class IntToken : public Token {
    public:
        short bit;
        uint64_t value;
        bool  isSigned;

        IntToken(uint64_t _value, bool _isSigned, short _bit) :Token(Tag::Integer) { value = _value; isSigned = _isSigned; bit = _bit; }
        string toString() { return "Integer: {0}"_format(value); }
    };

    // Floating Constant
    class FloatToken : public Token {
    public:
        short bit;
        long double value;

        FloatToken(long double _value, short _bit) :Token(Tag::Floating) { value = _value; bit = _bit; }
        string toString() { return "Floating: {0}"_format(value); }
    };

    // Charater Constant
    class CharToken : public Token {
    public:
        uint64_t value;

        CharToken(uint64_t _value) :Token(Tag::Character) { value = _value; }
        string toString() { return "Character: '{0}', value={1}"_format((char)value, value); }
    };

    // String Literals
    class StringToken : public Token {
    public:
        string value;

        StringToken(string _value) : Token(Tag::StringLiteral) { value = _value; }
        string toString() { return "String literal: \"{0}\""_format(value); }
    };
};

#endif
