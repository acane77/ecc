#ifndef _MIYUKI_PPLEX_H
#define _MIYUKI_PPLEX_H

#include "lex.h"

namespace Miyuki::Lex {

    DEFINE_SHARED_PTR(PreprocessorLexer)

    class PreprocessorLexer : public Lexer {
        uint16_t lexingContent = LexingContent::DefaultContent;

        TokenPtr scanKeywordOrIdentifier(string& word) override;
        bool eatCommentAndSpaces() override;
        TokenPtr scanPPNumber();
    public:
        void setLexingContent(uint16_t lexingContent) { PreprocessorLexer::lexingContent = lexingContent; }

    public:
        enum LexingContent: uint16_t {
            DefaultContent = 1,     // Normal code
            Preprocessing = 1 << 1, // Line start with a #
            Include = (1 << 2) | Preprocessing // Line start with a #include
        };

        explicit PreprocessorLexer() : Lexer() {  }
        TokenPtr scan() override;
    };


}

#endif