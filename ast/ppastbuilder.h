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

        explicit PreprocessorASTBuilder(const TokenSequencePtr& _src) : src(move(_src)) { next(); }

        // NOTE: here we rewrite because we need to parse AST for #if statement
        //       we get token from cache
        // get next token from cache
        TokenPtr next() override;

        void parse() override;

        // Test if parse is success
        //   for #if directive, it only build AST for expression, and due to this reason,
        //   we cannot check if it is in line with the grammar after the expressions that have been read,
        //   so we should check it manually, and if it next token is EOF, wo can refer than all tokens have
        //   been read, so it mean parsing is success, otherwise failed.
        bool success() { return next()->is(-1); }

    };

}

#endif