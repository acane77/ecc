#include "pplex.h"

namespace Miyuki::Lex {

    TokenPtr PreprocessorLexer::scan() {
        for (readch(); ; readch()) {
            if (peak == Tag::EndOfFile) return make_shared<Token>(Tag::EndOfFile);
            else if (peak == ' ' || peak == '\t' || peak == '\r') continue;
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
            else break;
        }

        Token::startColumn = M_fr->getColumn();

        /// Identifier, Keywords, String & Character Literal
        if (isalpha(peak) || peak == '_' || peak == '\\' || peak == '\'' || peak == '"')
            return scanIdentifierAndStringCharacterLiteral();

        // Header Name
        if ( lexingContent == LexingContent::Include &&  ( peak == '\"' || peak == '<' ) ) {
            char endChar = peak == '<' ? '>' : '\"';
            string strseq;
            for (readch(); peak != endChar ; readch()) {
                if (peak == '\r' || peak == '\n')  { throw SyntaxError("missing terminal character '{0}'"_format(endChar) ); }
                strseq += peak;
            }
            return make_shared<HeaderToken>(strseq);
        }

        // 

        // Punctuators
        return scanPunctuators();
    }

    TokenPtr PreprocessorLexer::scanKeywordOrIdentifier(string &word) {
        return make_shared<WordToken>(word);
    }
}