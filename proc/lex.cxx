#include "token.h"
#include "lex.h"
#include <format.h>
#include <climits>

namespace Miyuki::Proc {

    using namespace fmt;

    Lexer::Lexer(FileReadPtr frptr) {
        M_fr = frptr;
    }

    TokenPtr Lexer::scan() {
        // Eat comments and wite spaces
        for (readch(); ; readch()) {
            if (peak == Tag::EndOfFile) return make_shared<Token>(Tag::EndOfFile);
            else if (peak == ' ' || peak == '\t' || peak == '\r' || peak == '\n') continue;
            else if (peak == '/') {
                readch();
                if (peak == '/') {
                    for (; ; readch()) {
                        if (peak == '\n')  break;
                        if (peak == Tag::EndOfFile)   return make_shared<Token>(Tag::EndOfFile);
                    }
                }
                else if (peak == '*') {
                    readch();
                    for (; ; readch()) {
                        if (peak == Tag::EndOfFile)   throw SyntaxError("Comment not closed.");
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
                    retract();
                    break;
                }
            }
            else
                break;
        }

        Token::startColumn = M_fr->getColumn();

        // Integer
        long double floating;
        uint64_t intValue = 0;
        int ary;
        bool hasUnsignedSuffix = false;
        bool hasLongSuffix = false;
        bool hasLongLongSuffix = false;
        bool hasFloatingSuffix = false;
        bool suffixInvalid = false;
        string suffix = "";

        if (isdigit(peak)) {
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
                for (; isalpha(peak) ; readch()) {
                    suffix += peak;
                    if (suffixInvalid) continue;
                    if ((peak == 'u' || peak == 'U') && !hasUnsignedSuffix)
                        hasUnsignedSuffix = true;
                    else if ((peak == 'l' || peak == 'L') && !(hasLongLongSuffix || hasLongSuffix)) {
                        char save = peak; readch();
                        if (peak == save) { suffix += peak; hasLongLongSuffix = true; }
                        else { retract(); hasLongSuffix = true; }
                    }
                    else suffixInvalid = true;
                }

                if (suffixInvalid) throw SyntaxError("Invalid integer suffix '{0}'."_format(suffix));

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
            readch(); long double decimalPart;
            if (intValue == 0 && ary == 8) ary = 10;
            if (ary == 8)  throw SyntaxError("Invalid number.");
            else if (ary == 10) {
                long double fraction = 0.0, factor = 0.1;
                int exponment = 0;
                for (; isdigit(peak); readch()) {
                    fraction = fraction + (peak - '0') * factor;
                    factor = factor / 10;
                }
                if (peak == 'e' || peak == 'E')  { floating = fraction;  goto add_exponment; }
                decimalPart = fraction;
            }
            else if (ary == 16) {
                int fraction = 0, exponment = 0;  //faction * 1/16
                bool positive = true;
                for (; isdigit(peak) || (peak >= 'A' && peak <= 'F') || (peak >= 'a' && peak <= 'f'); readch() )
                    fraction = fraction * 16 + (isdigit(peak) ? peak - '0' : 10 + (peak >= 'A' && peak <= 'F' ? peak - 'A' : peak - 'a'));
                if (peak != 'p' && peak != 'P') throw SyntaxError("hexadecimal floating constants require an exponent");
                if (readch(); peak == '-' || peak == '+') { positive = peak == '+'; readch(); }
                for (; isdigit(peak); readch())
                    exponment = exponment * 10 + peak - '0';
                if (positive) decimalPart = (long double)fraction / 16 * (1 << exponment);
                else          decimalPart = (long double)fraction / 16 / (1 << exponment);
            }
            floating = ((long double)intValue) + decimalPart;
add_floating_suffix:
            // floting-constant suffix
            for (; isalpha(peak) ; readch()) {
                suffix += peak;
                if (suffixInvalid) continue;
                if ((peak == 'f' || peak == 'F') && !hasFloatingSuffix)
                    hasFloatingSuffix = true;
                else if ((peak == 'l' || peak == 'L') && !hasLongSuffix)
                    hasLongSuffix = true;
                else suffixInvalid = true;
            }

            if (suffixInvalid) throw SyntaxError("Invalid floating constant suffix '{0}'."_format(suffix));

            short bit;
            if (hasFloatingSuffix) bit = 32;
            else if (hasLongSuffix) bit = 64;
            else bit = (*((uint64_t *)(&floating)) >> 32) ? 32 : 64;

            return make_shared<FloatToken>(floating, bit);
        }

        return make_shared<Token>(peak);
    }

    Lexer::~Lexer() {

    }
}
