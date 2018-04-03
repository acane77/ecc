#include "token.h"
#include "lex.h"
#include "common/defines.h"
#include <format.h>
#include <climits>

namespace Miyuki::Lex {

    using namespace fmt;

    Lexer::Lexer() {
        M_sm = make_shared<SourceManager>();
    }

    TokenPtr Lexer::scan() {
        // Eat comments and wite spaces
        if (eatCommentAndSpaces()) return make_shared<Token>(Tag::EndOfFile);

        Token::startColumn = getColumn();

        /// Integer & Floating constant
        if (isdigit(peak) || peak == '.') {
            TokenPtr ptr = scanIntegerAndFloating();
            if (ptr) return ptr;
        }

        /// Identifier, Keywords, String & Character Literal
        if (isalpha(peak) || peak == '_' || peak == '\\' || peak == '\'' || peak == '"')
            return scanIdentifierAndStringCharacterLiteral();

        // Punctuators
        return scanPunctuators();
    }

    Lexer::~Lexer() {

    }

    char Lexer::_getCharFromSlash() {
        short chr;
        if (peak == '\\') {
            readch();
            if (peak == '\'' || peak == '\"' || peak == '?' || peak == '\\') chr = peak;
            else if (peak == 'a') chr = '\a';
            else if (peak == 'b') chr = '\b';
            else if (peak == 'f') chr = '\f';
            else if (peak == 'n') chr = '\n';
            else if (peak == 'r') chr = '\r';
            else if (peak == 't') chr = '\t';
            else if (peak == 'v') chr = '\v';
            else if (peak >= '0' && peak <= '7') {
                // octal-digit
                short x = peak - '0'; readch();
                if (peak >= '0' && peak <= '7') {
                    x = x * 8 + peak - '0'; readch();
                    if (peak >= '0' && peak <= '7') {
                        x = x * 8 + peak - '0';
                    }
                    else retract();
                }
                else retract();
                chr = x;
            }
            else if (peak == 'x') {
                // hexadecimal-escape-sequence
                short hex = 0;
                readch();
                if (!(isdigit(peak) || (peak >= 'a' && peak <= 'f') || (peak >= 'A' && peak <= 'D')))
                    diagError("Invalid hexadecimal-escape-sequence");
                for (; isdigit(peak) || (peak >= 'a' && peak <= 'f') || (peak >= 'A' && peak <= 'D'); readch()) {
                    hex = hex * 16 + peak - '0';
                }
                retract();
                chr = hex;
            }
            else diagError("Invalid escape sequence");
        }
        else chr = peak;
        return chr;
    }

    bool Lexer::eatCommentAndSpaces() {
        for (readch(); ; readch()) {
            if (peak == Tag::EndOfFile) return true;
            else if (peak == ' ' || peak == '\t' || peak == '\r' || peak == '\n') continue;
            else if (peak == '/') {
                readch();
                if (peak == '/') {
                    for (; ; readch()) {
                        if (peak == '\n')  break;
                        if (peak == Tag::EndOfFile)   return true;
                    }
                }
                else if (peak == '*') {
                    readch();
                    for (; ; readch()) {
                        if (peak == Tag::EndOfFile)   diagError("Comment not closed.");
                        if (peak == '*') {
                            readch();
                            if (peak == '/') break;
                            else             retract();
                        }
                    }
                }
                else {
                    // if code runs here, it represent that the '/' is as a divide instead of a comment
                    // So break here
                    retract(); peak = '/';
                    break;
                }
            }
            else break;
        }
        return false;
    }

    TokenPtr Lexer::scanPunctuators() {
            //  -> -- -=
        int singelPunc = peak;
        if (peak == '-') {  readch();
            if (peak == '>') return make_shared<Token>(Tag::PointerAccess);
            if (peak == '-') return make_shared<Token>(Tag::Decrease);
            if (peak == '=') return make_shared<Token>(Tag::SubAssign);
            retract();
        }
            // ++
        else if (peak == '+'){  readch();
            if (peak == '+') return make_shared<Token>(Tag::Increase);
            if (peak == '=') return make_shared<Token>(Tag::AddAssign);
            retract();
        }
            // << <<= <= <:
        else if (peak == '<') {  readch();
            if (peak == '<') {  readch();
                if (peak == '=')  return make_shared<Token>(Tag::LeftShiftAssign);
                retract(); return make_shared<Token>(Tag::LeftShift);
            }
            else if (peak == '=') return make_shared<Token>(Tag::LessThanEqual);
            else if (peak == ':') return make_shared<Token>(Tag::LessThanColon);
            else if (peak == '%') return make_shared<Token>(Tag::LessThanMod);
            retract();
        }
            // >> >>= >=
        else if (peak == '>') {  readch();
            if (peak == '>') {  readch();
                if (peak == '=')  return make_shared<Token>(Tag::RightShiftAssign);
                retract(); return make_shared<Token>(Tag::RightShift);
            }
            else if (peak == '=') return make_shared<Token>(Tag::GreaterThanEqual);
            retract();
        }
            // ==
        else if (peak == '=') {  readch();
            if (peak == '=') return make_shared<Token>(Tag::Equal);
            retract();
        }
            // !=
        else if (peak == '!') {  readch();
            if (peak == '=') return make_shared<Token>(Tag::NotEqual);
            retract();
        }
            // &&
        else if (peak == '&') {  readch();
            if (peak == '&') return make_shared<Token>(Tag::And);
            if (peak == '=') return make_shared<Token>(Tag::BitwiseAndAssign);
            retract();
        }
            // ||
        else if (peak == '|') {  readch();
            if (peak == '|') return make_shared<Token>(Tag::Or);
            if (peak == '=') return make_shared<Token>(Tag::BitwiseOrAssign);
            retract();
        }
            // ...
        else if (peak == '.') {  readch();
//maybe_an_ellipsis:
            if (peak == '.') {  readch();
                if (peak == '.') return make_shared<Token>(Tag::Ellipsis);
                retract(); diagError("invalid token '..'");
            }
            retract();
        }
            // *=
        else if (peak == '*') {  readch();
            if (peak == '=') return make_shared<Token>(Tag::MulpileAssign);
            retract();
        }
            // /=
        else if (peak == '/') {  readch();
            if (peak == '=') return make_shared<Token>(Tag::DivideAssign);
            retract();
        }
            // %=  %: %:%: %>
        else if (peak == '%') {  readch();
            if (peak == '=') return make_shared<Token>(Tag::ModAssign);
            if (peak == '>') return make_shared<Token>(Tag::ModGreaterThan);
            if (peak == ':' ) {  readch();
                if (peak == '%') {
                    if ((readch(), peak == ':')) return make_shared<Token>(Tag::ModColonDouble);
                    else diagError("Invalid token");
                }
                else return make_shared<Token>(Tag::ModColon);
            }
            retract();
        }
            // ##
        else if (peak == '#') {  readch();
            if (peak == '#') return make_shared<Token>(Tag::DoubleSharp);
            retract();
        }
            // ^=
        else if (peak == '^') {  readch();
            if (peak == '=') return make_shared<Token>(Tag::XorAssign);
            retract();
        }
            // other single-character punctuators
        return make_shared<Token>(singelPunc);
    }

    TokenPtr Lexer::scanKeywordOrIdentifier(string &word) {
        if (word == "alignof")  return make_shared<WordToken>(Tag::Alignof, word);
        if (word == "auto")  return make_shared<WordToken>(Tag::Auto, word);
        if (word == "asm")  return make_shared<WordToken>(Tag::Asm, word);
        if (word == "break")  return make_shared<WordToken>(Tag::Break, word);
        if (word == "case")  return make_shared<WordToken>(Tag::Case, word);
        if (word == "char")  return make_shared<WordToken>(Tag::Char, word);
        if (word == "const")  return make_shared<WordToken>(Tag::Const, word);
        if (word == "continue")  return make_shared<WordToken>(Tag::Continue, word);
        if (word == "default")  return make_shared<WordToken>(Tag::Default, word);
        if (word == "do")  return make_shared<WordToken>(Tag::Do, word);
        if (word == "double")  return make_shared<WordToken>(Tag::Double, word);
        if (word == "else")  return make_shared<WordToken>(Tag::Else, word);
        if (word == "enum")  return make_shared<WordToken>(Tag::Enum, word);
        if (word == "extern")  return make_shared<WordToken>(Tag::Extern, word);
        if (word == "float")  return make_shared<WordToken>(Tag::Float, word);
        if (word == "for")  return make_shared<WordToken>(Tag::For, word);
        if (word == "goto")  return make_shared<WordToken>(Tag::Goto, word);
        if (word == "if")  return make_shared<WordToken>(Tag::If, word);
        if (word == "inline")  return make_shared<WordToken>(Tag::Inline, word);
        if (word == "int")  return make_shared<WordToken>(Tag::Int, word);
        if (word == "long")  return make_shared<WordToken>(Tag::Long, word);
        if (word == "register")  return make_shared<WordToken>(Tag::Register, word);
        if (word == "restrict")  return make_shared<WordToken>(Tag::Restrict, word);
        if (word == "return")  return make_shared<WordToken>(Tag::Return, word);
        if (word == "short")  return make_shared<WordToken>(Tag::Short, word);
        if (word == "signed")  return make_shared<WordToken>(Tag::Signed, word);
        if (word == "sizeof")  return make_shared<WordToken>(Tag::Sizeof, word);
        if (word == "static")  return make_shared<WordToken>(Tag::Static, word);
        if (word == "struct")  return make_shared<WordToken>(Tag::Struct, word);
        if (word == "switch")  return make_shared<WordToken>(Tag::Switch, word);
        if (word == "typedef")  return make_shared<WordToken>(Tag::Typedef, word);
        if (word == "union")  return make_shared<WordToken>(Tag::Union, word);
        if (word == "unsigned")  return make_shared<WordToken>(Tag::Unsigned, word);
        if (word == "void")  return make_shared<WordToken>(Tag::Void, word);
        if (word == "volatile")  return make_shared<WordToken>(Tag::Volatile, word);
        if (word == "while")  return make_shared<WordToken>(Tag::While, word);
        return make_shared<WordToken>(word);
    }

    TokenPtr Lexer::scanIdentifierAndStringCharacterLiteral() {
        // Identifiers and Keywords, string-literal, character
        uint32_t encoding = Encoding::ASCII;
        if (isalpha(peak) || peak == '_' || peak == '\\') {
            string word = "";
            for (; isalnum(peak) || peak == '_' || peak == '\\'; readch()) {
                if (peak == '\\') {
                    diagError("My implementation does not support universal character name.");
                }
                word += peak;
            }
            // Test if it is prefix of character
            if (peak == '\"' || peak == '\'') {
                bool thisTokenIsAIdentifier = false;
                if (word == "u") encoding = Encoding::u;
                else if (word == "L") encoding = Encoding::L;
                else if (word == "U") encoding = Encoding::U;
                else if (word == "u8" && peak == '\"') encoding = Encoding::u8; // for string-literal only
                else thisTokenIsAIdentifier = true;
                if (!thisTokenIsAIdentifier)
                    // scan string-literal and character
                    return scanStringCharacterLiteral(encoding);
            }
            retract();
            return scanKeywordOrIdentifier(word);
        }
        else if (peak == '\"' || peak == '\'')
            return scanStringCharacterLiteral(encoding);

        assert( false && "This is not a (Identifiers and Keywords, string-literal, character)" );
    }

    TokenPtr Lexer::scanStringCharacterLiteral(uint32_t encoding) {
        if (peak == '\'' || peak == '"') {
            int quotationMark = peak;
            string chrseq;
            readch();
            for (; ; readch()) {
                if (peak == quotationMark) break;
                else if (peak == '\r' || peak == '\n') diagError("Unexpected new-line");
                else if (peak == -1) diagError("Unexpected end-of-file");
                else chrseq += _getCharFromSlash();
            }

            return quotationMark == '\"' ?
                   static_pointer_cast<Token>(make_shared<StringToken>(chrseq, encoding)) :
                   static_pointer_cast<Token>(make_shared<CharToken>(chrseq, encoding));
        }

        assert( false && "this is not a string-literal nor character" );
    }

    TokenPtr Lexer::scanIntegerAndFloating() {
        // Integer and floating constant
        FloatingLiteralType floating;
        // Integer of integer part of floating
        uint64_t intValue = 0;
        // Ary of the integer (or integer part of floating)
        int ary = 10;
        // Integer has unsigbed suffix (u)
        bool hasUnsignedSuffix = false;
        // Integer or floating has long suffix (L)
        bool hasLongSuffix = false;
        // Integer has long long suffix (LL)
        bool hasLongLongSuffix = false;
        // Test if there's a floating suffix (F)
        bool hasFloatingSuffix = false;
        // Floating or Integer suffix is vilid
        bool suffixInvalid = false;
        // Represent if code goes to floating test directly instead of enterring to integer
        bool gotoFloatingDirectly = true;
        // Store suffix for feature use
        string suffix;

        if (isdigit(peak)) {
            gotoFloatingDirectly = false;
            if (peak != '0') {
                // decimal-constant
                ary = 10;
                for (; isdigit(peak); readch())
                    intValue = intValue * ary + peak - '0';
            }
            else {
                readch();
                if (peak == 'x' || peak == 'X') {
                    // hexadecimal-constant
                    ary = 16;
                    for (readch(); isdigit(peak) || (peak >= 'A' && peak <= 'F') || (peak >= 'a' && peak <= 'f'); readch() )
                        intValue = intValue * ary + (isdigit(peak) ? peak - '0' : 10 + (peak >= 'A' && peak <= 'F' ? peak - 'A' : peak - 'a'));
                }
                else {
                    // octal-constant
                    ary = 8;
                    for (; isdigit(peak); readch())
                        intValue = intValue * ary + peak - '0';
                }
            }

            // if next char is dot it is a floting-constant
            if (peak == '.')  ; // do nothing
                // floating (hexadecimal)
            else if (peak == 'e') {
                floating = intValue;
add_exponment:
                int exponment = 0;  //faction * 1/16
                bool positive = true;
                if (readch(); peak == '-' || peak == '+') { positive = peak == '+'; readch(); }
                if (!isdigit(peak)) diagError("exponent has no digits");
                for (; isdigit(peak); readch())
                    exponment = exponment * 10 + peak - '0';
                for (int i=0; i<exponment; i++) {
                    if (positive) floating *= ary;
                    else floating /= ary;
                }
                // Add floating-constant suffix below
                goto add_floating_suffix;
            }
                // integer-suffix
            else  {
                for (; isalnum(peak) ; readch()) {
                    suffix += (char)peak;
                    if (suffixInvalid) continue;
                    if ((peak == 'u' || peak == 'U') && !hasUnsignedSuffix)
                        hasUnsignedSuffix = true;
                    else if ((peak == 'l' || peak == 'L') && !(hasLongLongSuffix || hasLongSuffix)) {
                        char save = (char)peak; readch();
                        if (peak == save) { suffix += (char)peak; hasLongLongSuffix = true; }
                        else { retract(); hasLongSuffix = true; }
                    }
                    else if ((peak == 'f' || peak == 'F') && !hasFloatingSuffix)
                        hasFloatingSuffix = true;
                    else suffixInvalid = true;
                }

                if (suffixInvalid) diagError("Invalid integer suffix '{0}'."_format(suffix));
                if (hasFloatingSuffix) {
                    if (hasUnsignedSuffix || hasLongLongSuffix || hasLongSuffix) diagError("Invalid floating constant suffix '{0}'."_format(suffix));
                    floating = intValue;
                    goto convert_to_floating_here;
                }
                retract();
                it_must_be_an_integer:
                short bit;
                if (hasLongLongSuffix) bit = 64;
                else if (hasLongSuffix) bit = 32;
                else if ((hasUnsignedSuffix && intValue < INT16_MAX) || (!hasUnsignedSuffix && intValue < INT16_MAX / 2) ) bit = 16;
                else bit = 32;
                return make_shared<IntToken>(intValue, hasUnsignedSuffix, bit);
            }

        }

        // Floating-constant (decimal)
        if (peak == '.') {
            readch(); FloatingLiteralType decimalPart;
            if (peak == '.') {
                // why retract here? if here is .. it reads two more characters than
                // nessesarry, so retract is required.
                // if !gotoFloatingDirectly source like '123...', integer + ...
                if (!gotoFloatingDirectly)  { retract(); retract(); goto it_must_be_an_integer; }
                    // it may be ..., .. is also possible
                    // retract is also required but only once.
                    // we can manually set peak = '.' because we know what peak should be
                else  { retract(); peak = '.'; return nullptr; };
            }
            if (gotoFloatingDirectly && !(isdigit(peak))) {
                // floating like .12 = 0.12, it must be a decimal number.
                // assign directly just becsuse peak must be dot
                retract(); peak = '.'; return nullptr;
            }

            if (intValue == 0 && ary == 8) ary = 10;
            if (ary == 8)  diagError("Invalid number.");
            else if (ary == 10) {
                FloatingLiteralType fraction = 0.0, factor = 0.1;
                int exponment = 0;
                for (; isdigit(peak); readch()) {
                    fraction = fraction + (peak - '0') * factor;
                    factor = factor / 10;
                }
                if (peak == '.') diagError("too many decimal points in number");
                if (peak == 'e' || peak == 'E')  { floating = intValue + fraction;  goto add_exponment; }
                decimalPart = fraction;
            }
            else if (ary == 16) {
                int fraction = 0, exponment = 0;  //faction * 1/16
                bool positive = true;
                for (; isdigit(peak) || (peak >= 'A' && peak <= 'F') || (peak >= 'a' && peak <= 'f'); readch() )
                    fraction = fraction * 16 + (isdigit(peak) ? peak - '0' : 10 + (peak >= 'A' && peak <= 'F' ? peak - 'A' : peak - 'a'));
                if (peak == '.') diagError("too many decimal points in number");
                if (peak != 'p' && peak != 'P') diagError("hexadecimal floating constants require an exponent");
                if (readch(); peak == '-' || peak == '+') { positive = peak == '+'; readch(); }
                for (; isdigit(peak); readch())
                    exponment = exponment * 10 + peak - '0';
                if (positive) decimalPart = (FloatingLiteralType)fraction / 16 * (1 << exponment);
                else          decimalPart = (FloatingLiteralType)fraction / 16 / (1 << exponment);
            }
            floating = ((FloatingLiteralType)intValue) + decimalPart;
add_floating_suffix:
            // floting-constant suffix
            for (; isalnum(peak) ; readch()) {
                suffix += peak;
                if (suffixInvalid) continue;
                if ((peak == 'f' || peak == 'F') && !(hasFloatingSuffix || hasLongSuffix))
                    hasFloatingSuffix = true;
                else if ((peak == 'l' || peak == 'L') && !(hasFloatingSuffix || hasLongSuffix))
                    hasLongSuffix = true;
                else suffixInvalid = true;
            }
convert_to_floating_here:
            if (suffixInvalid) diagError("Invalid floating constant suffix '{0}'."_format(suffix));

            short bit;
            if (hasFloatingSuffix) bit = 32;
            else if (hasLongSuffix) bit = 64;
            else bit = (*((uint64_t *)(&floating)) >> 32) ? 32 : 64;

            if (peak == '.') diagError("too many decimal points in number");

            retract();
            return make_shared<FloatToken>(floating, bit);
        }

        assert( false && "this is neither integer nor floating token" );
    }

    void Lexer::eatAnyNonBlankChar() {
        for (; peak != ' ' && peak != '\n' && peak != '\r' && peak != '\t' && peak != -1; readch());
    }

    TokenPtr Lexer::getLexedInvalidToken() {
        assert( detectAnError && "invalid call of Lexer::getLexedInvalidToken()" );
        detectAnError = false;
        return make_shared<Token>(-1);
    }
}
