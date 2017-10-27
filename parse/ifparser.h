#ifndef _MIYUKI_IF_PARSER_H
#define _MIYUKI_IF_PARSER_H

#include "lex/lex.h"
#include <deque>
#include <iostream>

namespace Miyuki::Parse {

    using namespace Lex;
    // this file defines an interface for parser, and implement common operations

    DEFINE_SHARED_PTR(IParser)

    class ParseError : public exception {
        string msg;
        Lex::TokenPtr tok;
        bool warning;

    public:
        ParseError(string&& _msg, Lex::TokenPtr _tok, bool _warning) { msg = std::move(_msg); tok = _tok; warning = _warning; }
        ParseError(string& _msg, Lex::TokenPtr _tok, bool _warning) { msg = std::move(_msg); tok = _tok; warning = _warning; }

        const char * what() const noexcept override { return msg.c_str(); }
        const Lex::TokenPtr getToken() { return tok; }
        bool isWarning() { return warning; }
    };

    class IParser {

        LexerPtr M_lex;
        // lookahead token
        TokenPtr look;
        // thrown exceptions
        deque<ParseError> errors;

        struct CommonParserState {
            // if compiler meet error
            bool encountErrors = false;

        } commonParserState;

        enum {
            // max number of retractable tokens
            MaxRetractSize = 10
        };

        // Saved and retracted tokens
        TokenPtr tokens[MaxRetractSize];
        // Pointer to read token
        int     m_tsptr_r = -1;
        // Pointer to write token
        int     m_tsptr_w = -1;

                             /////  token matching //////
        // test match, if not, throw exception with msg
        void match(uint32_t term, string&& errmsg, TokenPtr& ptr);

        // get next token from lexer
        TokenPtr next();
        // Note: retract is 'put back' 1 token , not read the value of
        //       previous token.
        TokenPtr retract();

                              ///// error recovery //////
        // error-recovery flag
        enum {
            SkipUntilSemi = 1 >> 0,  // Stop at ';'
            KeepSpecifiedToken = 1 >> 1,  // When find specified token ,keep it
            ConsumeSpecifiedToken = 1 >> 2, // When find specified token , consume it
            SkipUntilEndOfFile = 1 >> 3 // Stop at EOF
        };

        // test recovery flag
        bool hasRecoveryFlag(uint32_t a, uint32_t b) { return a & b; }

        // skip tokens
        // if returns false means we do not find token we want
        // if returns true means we've found tokens we need
        // skipUntil usage:  if next token isn't what we want, then specify tokens and skipping
        //    flags, if return true, then let's continue parsing, otherwise bail out
        virtual bool skipUntil(deque<uint32_t> toks, uint32_t flag) = 0;

        // add error statement prepare for output
        inline void diagError( string&& errmsg, TokenPtr& ptr ) { errors.push_back(ParseError(errmsg, ptr, false)); }

        // add warning statement prepare for output
        // warning information can be disabled by flag
        inline void diagWarning( string&& errmsg, TokenPtr& ptr ) { errors.push_back(ParseError(errmsg, ptr, true)); }

    public:
        explicit IParser(LexerPtr lex) { M_lex = std::move(lex); }

        // parse source file
        virtual void parse() = 0;

        // report error from exception
        virtual void reportError(std::ostream& os);

    };

};

#endif