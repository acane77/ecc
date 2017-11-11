#include "pplex.h"

namespace Miyuki::Lex {

    TokenPtr PreprocessorLexer::scan() {
        if (eatCommentAndSpaces())  return make_shared<Token>(Tag::EndOfFile);

        Token::startColumn = M_sm->getColumn();

        // Header Name
        if ( (lexingContent == LexingContent::Include) &&  ( peak == '\"' || peak == '<' ) ) {
            char endChar = peak == '<' ? '>' : '\"';
            string strseq;
            for (readch(); peak != endChar ; readch()) {
                if (peak == '\r' || peak == '\n')  { throw SyntaxError("missing terminal character '{0}'"_format(endChar) ); }
                strseq += peak;
            }
            return make_shared<HeaderToken>(strseq);
        }

        /// Identifier, Keywords, String & Character Literal
        if (isalpha(peak) || peak == '_' || peak == '\\' || peak == '\'' || peak == '"')
            return scanIdentifierAndStringCharacterLiteral();

        // PPNumber
        if ( (lexingContent & LexingContent ::Preprocessing) && ( isdigit(peak) || peak == '.' ) ) {
            int peakSav = peak;  readch(); int nextChr = peak; retract(); peak = peakSav;
            if (nextChr == '.') goto it_is_a_punctuator;
            TokenPtr ptr = scanPPNumber();
            if (ptr) return ptr;
        }

        // Other literal tokens
        if ( (lexingContent & LexingContent ::DefaultContent) && (isdigit(peak) || peak == '.') ) {
            int prevChar = peak; readch();
            if (prevChar == '.' && !isdigit(peak)) {  retract(); peak = prevChar; goto it_is_a_punctuator; }
            string charseq = "";
            charseq += (char)prevChar;
            for (; isalnum(peak) || peak == '.' ; readch() )
                charseq += peak;
            retract();
            return make_shared<PPLiteralToken>(charseq);
        }
it_is_a_punctuator:
        // Punctuators
        TokenPtr puncTok = scanPunctuators();
        if (puncTok->is('#')) lexingContent = LexingContent ::Preprocessing;
        else if (puncTok->is('\n')) lexingContent = LexingContent ::DefaultContent;
        return puncTok;
    }

    TokenPtr PreprocessorLexer::scanKeywordOrIdentifier(string &word) {
        return make_shared<WordToken>(word);
    }

    TokenPtr PreprocessorLexer::scanPPNumber() {
        bool noIntPart = !isdigit(peak), noFloatingPart;
        FloatingType fracPart = 0.0, fraction = 0.1;
        for ( ; isdigit(peak) ; readch() )
            fracPart = fracPart * 10 + (peak - '0');
        // Has no floating part, just go exponment
        if (peak != '.')  goto add_exponment;

        // scan floating-point part
        readch();  noFloatingPart = !isdigit(peak);
        for ( ; isdigit(peak) ; readch() , fraction = fraction / 10 )
            fracPart = fracPart + fraction * ( peak - '0' );
        // If has no int-part nor float-part means just a point,
        // so return null, and scan as-is.
        if (noFloatingPart && noIntPart)   return nullptr;
        // scan exponment part
add_exponment:
        int exp = 0;
        if (peak == 'e' || peak == 'E') {
            for (readch(); isdigit(peak); readch() )
                exp = exp * 10 + peak - '0';
            for (int i=0; i<exp; i++)
                fracPart = fracPart * 10;
        }
        else if (peak == 'p' || peak == 'P') {
            for (readch(); isdigit(peak); readch() )
                exp = exp * 10 + peak - '0';
            fracPart = fracPart * (1 << exp);
        }
        retract();
        return make_shared<PPNumberToken>(fracPart);
    }

    bool PreprocessorLexer::eatCommentAndSpaces() {
        for (readch(); ; readch()) {
            if (peak == Tag::EndOfFile) return true;
            else if (peak == ' ' || peak == '\t' || peak == '\r') continue;
            else if (peak == '/') {
                readch();
                if (peak == '/') {
                    for (; ; readch()) {
                        if (peak == '\n') { retract();  break; }
                        if (peak == Tag::EndOfFile)   return true;
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
            else break;
        }
        return false;
    }
}