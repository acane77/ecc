#ifndef _MIYUKI_LEXTOKEN_H
#define _MIYUKI_LEXTOKEN_H

#include "common/srcmgr.h"
#include "include.h"
#include "tags.h"
#include "common/ptrdef.h"
#include "common/defines.h"
#include "common/encoding.h"
#include "include/fmt/format.h"

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
        int startCol;
        int column;
        int row;
        int chrlen;
        const char * filenam;

        // characteristic information
        int32_t tag;

        // use for getting know about more information
        static Common::SourceManagerPtr flread;
        static int startColumn;

        explicit Token(int32_t _tag) { tag = _tag;
            /// FIXME: BUG!!!  position and length of token
            if (flread) {
                column = flread->getColumn(); row = flread->getRow() - 1; //row returned by lexer is next row nuber so minus 1
                filenam = flread->getCurrentFilename().c_str(); chrlen = column - startColumn;
                startCol = startColumn;
            } else filenam = "";
        }
        virtual string toString() {
            if (tag > 31 && tag < 127)  return "Sign: {0}"_format((char)tag);
            return "Token[tag={0}] {1}"_format(tag, toSourceLiteral() );
        }
        // generate string in source code (for preprocessor propose)
        virtual string toSourceLiteral();
        // test if this is _tag
        bool is(int32_t _tag) {

            // if _tag passes special kind, then compare
            if (_tag == Tag::Punctuator)  return tag >= Tag::Property::PunctuatorStart && tag <= Tag::Property::PunctuatorEnd;
            // expected token is a punctuator or keyword or EOF - using literal value comparsion
            // Type comparsion - using bitwise-or
            return ((_tag >= Tag::Property::PunctuatorStart && _tag <= Tag::Property::KeywordEnd) || _tag == Tag::EndOfFile || tag == Tag::EndOfFile) ? (_tag == tag) : (_tag & tag);
        }
        bool isNot(int32_t _tag) { return !is(_tag); }
        bool isRelationshipOperators() {
            return is(Tag::LessThanEqual) || is(Tag::GreaterThanEqual) || is(Tag::Equal) || is(Tag::NotEqual) || is('>') || is('<') || is('!');
        }

        // type conversion (used by AST eval)
        virtual FloatingType toFloat() { assert( false && "this token cannot convert to floating point" ); }
        virtual IntegerType toInt() { assert( false && "this token cannot convert to integer" ); }
        virtual void setValue(FloatingType f) { assert( false && "this token cannot set value of floating point" ); }
        virtual void setValue(IntegerType i) { assert( false && "this token cannotset value of integer" ); }

        // other
        void copyAdditionalInfo(const TokenPtr& otherToken);
    };

    // Identifier & Keyword & Enumration
    class WordToken : public Token {
    public:
        string name;

        // For Identifier
        explicit WordToken(string&& _name) :Token(Tag::ID) { name = _name; }
        explicit WordToken(string& _name) :Token(Tag::ID) { name = move(_name); }

        // For keywords
        WordToken(int32_t _tag, string&& _name) :Token(_tag) { name = _name; }
        WordToken(int32_t _tag, string& _name) :Token(_tag) { name = move(_name); }

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
        IntegerType signedValue;

        IntToken(uint64_t _value, bool _isSigned = true, short _bit = 32) :Token(Tag::Integer) { value = _value; isSigned = _isSigned; bit = _bit; signedValue = value; }
        string toString() override { return "Integer: {0}"_format(value); }
        string toSourceLiteral() override { return "{0}"_format(value); }
        IntegerType toInt() override { return signedValue; }
        FloatingType toFloat() override { return (FloatingType)signedValue; }
        void setValue(IntegerType v) override { value = (uint64_t)v; signedValue = v; }
    };

    // Floating Constant
    class FloatToken : public Token {
    public:
        short bit;
        FloatingType value;

        FloatToken(FloatingType _value, short _bit) :Token(Tag::Floating) { value = _value; bit = _bit; }
        string toString() override { return "Floating: {0}"_format(value); }
        string toSourceLiteral() override { return "{0}"_format(value); }

        FloatingType toFloat() override { return value; }
        IntegerType toInt() { return (IntegerType)value; }
        void setValue(FloatingType f) { value = f; }
        void setValue(IntegerType i) { value = (FloatingType)i; }
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

        explicit CharToken(string&& _charseq, int32_t enc) :Token(Tag::Character), encoding(enc), charseq(std::move(_charseq)) {
            _setIntValue();
        }
        explicit CharToken(string& _charseq, int32_t enc) :Token(Tag::Character), encoding(enc), charseq(std::move(_charseq)) {
            _setIntValue();
        }
        string toString() override { return "Character({2}): '{0}', value={1}"_format((char)value, value, encoding.getEncodingString()); }
        string toSourceLiteral() override { return "{0}'{1}'"_format( encoding.getEncodingString(), charseq ); }

        IntegerType toInt() override { return (IntegerType)value; }
        FloatingType toFloat() override { return (FloatingType)value; }
    };

    // String Literals
    class StringToken : public Token {
    public:
        string value;
        Encoding encoding;

        explicit StringToken(string&& _value, int32_t enc) : Token(Tag::StringLiteral), encoding(enc) { value = _value; }
        explicit StringToken(string& _value, int32_t enc) : Token(Tag::StringLiteral), encoding(enc) { value = move(_value); }
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
        string toSourceLiteral() override { return "{0}"_format(value); }
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
