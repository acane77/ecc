#ifndef _MIYUKI_LEXTOKEN_H
#define _MIYUKI_LEXTOKEN_H

#include "common/srcmgr.h"
#include "include.h"
#include "tags.h"
#include "common/ptrdef.h"
#include "common/defines.h"
#include "common/encoding.h"
#include <format.h>

using namespace std;
using namespace fmt;

namespace Miyuki::Lex {

    using namespace Common;
    // This file defines tokens (ref ISO/IEC 9899:201x $A.1)

    DEFINE_SHARED_PTR(Token)
    DEFINE_SHARED_PTR(WordToken)
    DEFINE_SHARED_PTR(IntToken)
    DEFINE_SHARED_PTR(FloatToken)
    DEFINE_SHARED_PTR(CharToken)
    DEFINE_SHARED_PTR(HeaderToken)
    DEFINE_SHARED_PTR(PPNumberToken)
    DEFINE_SHARED_PTR(PPLiteralToken)

    // Punctuator Strings
    extern const char * PunctuatorString[];

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
        static Common::SourceManagerPtr flread;
        static int startColumn;

        explicit Token(uint32_t _tag) { tag = _tag; if (flread) { column = flread->getColumn(); row = flread->getRow(); filenam = ""; chrlen = column - startColumn; } }
        virtual string toString() {
            if (tag > 31 && tag < 127)  return "Sign: {0}"_format((char)tag);
            return "Token[tag={0}] {1}"_format(tag, toSourceLiteral() );
        }
        // generate string in source code (for preprocessor propose)
        virtual string toSourceLiteral();
        // test if this is _tag
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
        string toSourceLiteral() override { return name; }
    };

    // Integer Constant
    class IntToken : public Token {
    public:
        short bit;
        uint64_t value;
        bool  isSigned;

        IntToken(uint64_t _value, bool _isSigned, short _bit) :Token(Tag::Integer) { value = _value; isSigned = _isSigned; bit = _bit; }
        string toString() override { return "Integer: {0}"_format(value); }
        string toSourceLiteral() override { return "{0}"_format(value); }
    };

    // Floating Constant
    class FloatToken : public Token {
    public:
        short bit;
        FloatingType value;

        FloatToken(FloatingType _value, short _bit) :Token(Tag::Floating) { value = _value; bit = _bit; }
        string toString() override { return "Floating: {0}"_format(value); }
        string toSourceLiteral() override { return "{0}"_format(value); }
    };

    // Charater Constant
    class CharToken : public Token {
        void _setIntValue() {
            if (!charseq.length()) throw SyntaxError("empty character constant");
            value = 0;
            for (int i=charseq.length() - 1; i>=0; i--)
                value = (value << 8) | charseq[i];
        }
    public:
        uint64_t value;
        Encoding encoding;
        string   charseq;

        explicit CharToken(string&& _charseq, uint32_t enc) :Token(Tag::Character), encoding(enc), charseq(std::move(_charseq)) {
            _setIntValue();
        }
        explicit CharToken(string& _charseq, uint32_t enc) :Token(Tag::Character), encoding(enc), charseq(std::move(_charseq)) {
            _setIntValue();
        }
        string toString() override { return "Character({2}): '{0}', value={1}"_format((char)value, value, encoding.getEncodingString()); }
        string toSourceLiteral() override { return "{0}'{1}'"_format( encoding.getEncodingString(), charseq ); }
    };

    // String Literals
    class StringToken : public Token {
    public:
        string value;
        Encoding encoding;

        explicit StringToken(string&& _value, uint32_t enc) : Token(Tag::StringLiteral), encoding(enc) { value = _value; }
        explicit StringToken(string& _value, uint32_t enc) : Token(Tag::StringLiteral), encoding(enc) { value = move(_value); }
        string toString() override { return "String literal({1}): \"{0}\""_format(value, encoding.getEncodingString()); }
        string toSourceLiteral() override { return "{0}\"{1}\""_format( encoding.getEncodingString(), value ); }
    };

    // Prepressor Header Names
    class HeaderToken : public Token {
    public:
        string name;

        explicit HeaderToken(string && _name) : Token(Tag::HeaderName) { name = _name; }
        explicit HeaderToken(string & _name) : Token(Tag::HeaderName) { name = move(_name); }
        string toString() override { return "Header name: {0}"_format(name); }
        string toSourceLiteral() override { assert(false && "HeaderName cannot reprint to source"); }
    };

    // Prepressor Numbers
    class PPNumberToken : public Token {
    public:
        FloatingType value;

        explicit PPNumberToken(FloatingType _value) : Token(Tag::PPNumber) { value = _value; }
        string toString() override { return "PP Number: {0}"_format(value); }
        string toSourceLiteral() override { assert(false && "PPNumber cannot reprint to source"); }
    };

    // Prepressor other as-is tokens
    class PPLiteralToken : public Token {
    public:
        string literal;

        explicit PPLiteralToken(string&& _literal) : Token(Tag::PPLiteral) { literal = _literal; }
        explicit PPLiteralToken(string& _literal) : Token(Tag::PPLiteral) { literal = move(_literal); }
        string toString() override { return "PP Literal: {0}"_format(literal); }
        string toSourceLiteral() override { return literal; }
    };
};

#endif
