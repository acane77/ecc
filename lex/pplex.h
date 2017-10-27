#ifndef _MIYUKI_PPLEX_H
#define _MIYUKI_PPLEX_H

#include "lex.h"

namespace Miyuki::Lex {

    class PreprocessorLexer : public Lexer {
        uint16_t lexingContent = LexingContent::DefaultContent;
    public:
        void setLexingContent(uint16_t lexingContent) { PreprocessorLexer::lexingContent = lexingContent; }

    public:
        enum LexingContent: uint16_t {
            DefaultContent = 0, Include = 1
        };

        explicit PreprocessorLexer(FileReadPtr fr) : Lexer(fr) {  }
        TokenPtr scanKeywordOrIdentifier(string& word) override;
        TokenPtr scan() override;
    };


}

#endif