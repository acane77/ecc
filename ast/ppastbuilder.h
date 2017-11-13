#ifndef _MIYUKI_PPASTBUILDER_H
#define _MIYUKI_PPASTBUILDER_H

#include "astbuilder.h"

// this files defines Preprocessor Abstract Syntax Tree Builder
// only used in preprocessor, (when parse if statement, dynamic create instance of this class
// and every expression in if statement create one instance)

namespace Miyuki::AST {

    using namespace Miyuki::Parse;

    class PreprocessorASTBuilder : public ASTBuilder {

        // token source: evaled token sequence
        TokenSequencePtr src;
        int              tokIndex = 0;

    public:

        explicit PreprocessorASTBuilder(const TokenSequencePtr& _src) : src(move(_src)) { }

        // NOTE: here we rewrite because we need to parse AST for #if statement
        //       we get token from cache
        // get next token from cache
        TokenPtr next() override;

        void parse() override;

    };

}

#endif