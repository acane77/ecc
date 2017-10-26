#ifndef _MIYUKI_LEXTOKEN_H
#define _MIYUKI_LEXTOKEN_H

#include "common/flread.h"
#include "include.h"
#include "tags.h"
#include "common/ptrdef.h"
#include <format.h>

using namespace std;
using namespace fmt;

namespace Miyuki::Lex {

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
        uint32_t tag;

        // use for getting know about more information
        static Common::FileReadPtr flread;
        static int startColumn;

        explicit Token(uint32_t _tag) { tag = _tag; if (flread) { column = flread->getColumn(); row = flread->getRow(); filenam = ""; chrlen = column - startColumn; } }
        virtual string toString() {
            if (tag > 31 && tag < 127)  return "Sign: {0}"_format((char)tag);
            return "Token[tag={0}]"_format(tag);
        }
        bool is(uint32_t _tag) {
            // expected token is a punctuator or keyword or EOF - using literal value comparsion
            // Type comparsion - using bitwise-or
            return ((_tag >= Tag::Property::PunctuatorStart && _tag <= Tag::Property::KeywordEnd) || _tag == Tag::EndOfFile) ? (_tag == tag) : (_tag & tag);
        }
    };

    // Identifier & Keyword & Enumration
    class WordToken : public Token {
    public:
        string name;

        // For Identifier
        explicit WordToken(string&& _name) :Token(Tag::ID) { name = _name; }
        explicit WordToken(string& _name) :Token(Tag::ID) { name = move(_name); }

        // For keywords
        WordToken(uint32_t _tag, string&& _name) :Token(_tag) { name = _name; }
        WordToken(uint32_t _tag, string& _name) :Token(_tag) { name = move(_name); }

        // For Enmeration
        void changeTypeToEnumeration() { tag = Tag::Enumeration; }

        string toString() override { return "{0}: {1}"_format(tag == Tag::ID ? "Identifier" : ( tag == Tag::Enumeration ? "Enumeration" : "Keyword" ),name); }
    };

    // Integer Constant
    class IntToken : public Token {
    public:
        short bit;
        uint64_t value;
        bool  isSigned;

        IntToken(uint64_t _value, bool _isSigned, short _bit) :Token(Tag::Integer) { value = _value; isSigned = _isSigned; bit = _bit; }
        string toString() override { return "Integer: {0}"_format(value); }
    };

    // Floating Constant
    class FloatToken : public Token {
    public:
        short bit;
        long double value;

        FloatToken(long double _value, short _bit) :Token(Tag::Floating) { value = _value; bit = _bit; }
        string toString() override { return "Floating: {0}"_format(value); }
    };

    // Charater Constant
    class CharToken : public Token {
    public:
        uint64_t value;

        explicit CharToken(uint64_t _value) :Token(Tag::Character) { value = _value; }
        string toString() override { return "Character: '{0}', value={1}"_format((char)value, value); }
    };

    // String Literals
    class StringToken : public Token {
    public:
        string value;

        explicit StringToken(string&& _value) : Token(Tag::StringLiteral) { value = _value; }
        explicit StringToken(string& _value) : Token(Tag::StringLiteral) { value = move(_value); }
        string toString() override { return "String literal: \"{0}\""_format(value); }
    };
};

#endif
