#ifndef _MIYUKI_IF_PARSER_H
#define _MIYUKI_IF_PARSER_H

#include "lex/lex.h"
#include "common/observe.h"
#include <deque>
#include <iostream>

namespace Miyuki::Parse {

    using namespace Lex;
    // this file defines an interface for parser, and implement common operations

    DEFINE_SHARED_PTR(IParser)

    typedef deque<TokenPtr> TokenSequence;
    typedef shared_ptr<TokenSequence> TokenSequencePtr;
    typedef TokenSequence::iterator TokenSequenceIter;

    extern std::ostream& operator << (std::ostream& os, const TokenSequence& tokenSeq);

    class ParseError : public exception {
        // normal error message
        string msg;
        // more info string
        string info;
        // suggest replace for invaid token
        string suggest;
        // error token
        Lex::TokenPtr tok;
        // show if this is a warning
        bool warning;

    public:
        const string &getInfo() const { return info; }
        void setInfo(const string &info) {  ParseError::info = info; }
        const string &getSuggest() const { return suggest; }
        void setSuggest(const string &suggest) { ParseError::suggest = suggest; }

    public:
        ParseError(string&& _msg, Lex::TokenPtr _tok, bool _warning) { msg = std::move(_msg); tok = _tok; warning = _warning; }
        ParseError(string& _msg, Lex::TokenPtr _tok, bool _warning) { msg = std::move(_msg); tok = _tok; warning = _warning; }

        const char * what() const noexcept override { return msg.c_str(); }
        const Lex::TokenPtr getToken() { return tok; }
        bool isWarning() { return warning; }
    };

    class PasreCannotRecoveryException : public exception {
    public:
        const char * what() const noexcept override { return "compilation failed due to errors."; }
    };

    class IParser : public IObserver {
    protected:
        LexerPtr M_lex;
        // lookahead token
        TokenPtr look;
        // thrown exceptions (common used by all instance)
        static deque<ParseError> errors;
        // error count (Note: errorCount != length of error as warning items is also in errors)
        static size_t errorCount;

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

        // Cache token
        void    cacheToken(TokenPtr tok);

                             /////  token matching //////
        // test match, if not, throw exception with msg
        void match(uint32_t term, string&& errmsg, TokenPtr& ptr);

        // get next token from lexer
        virtual TokenPtr next();
        // Note: retract is 'put back' 1 token , not read the value of
        //       previous token.
        virtual TokenPtr retract();
    public:
                              ///// error recovery //////
        // error-recovery flag
        enum RecoveryFlag : uint32_t {
            SkipUntilSemi = 1 >> 0,  // Stop at ';'
            KeepSpecifiedToken = 1 >> 1,  // When find specified token ,keep it
            ConsumeSpecifiedToken = 1 >> 2, // When find specified token , consume it
            SkipUntilEndOfFile = 1 >> 3 // Stop at EOF, if not found specified
        };

        // test recovery flag
        bool hasRecoveryFlag(uint32_t a, uint32_t b) { return a & b; }

        // skip tokens
        // if returns false means we do not find token we want
        // if returns true means we've found tokens we need
        // skipUntil usage:  if next token isn't what we want, then specify tokens and skipping
        //    flags, if return true, then let's continue parsing, otherwise bail out
        virtual bool skipUntil(const deque<int32_t>& toks, uint32_t flag);

        // add error statement prepare for output
        inline void diagError( string&& errmsg, TokenPtr ptr ) { errors.push_back(ParseError(errmsg, ptr, false)); errorCount++; }

        // add warning statement prepare for output
        // warning information can be disabled by flag
        inline void diagWarning( string&& errmsg, TokenPtr ptr ) { errors.push_back(ParseError(errmsg, ptr, true)); }

                              ////// parser state //////
        // called when parse done
        virtual void parseDone();

        // report error from exception
        virtual void reportError(std::ostream& os);

        explicit IParser() { instance = this; }
        explicit IParser(int doNotSetInstance) { }

        // parse source file
        virtual void parse() = 0;

        // observable
        void notify(uint32_t what) override;
        void registerObserver() { M_lex->getSourceManager()->addObserver(static_cast<IObserver*>(this)); }

        // static instance
        static IParser * instance;
    };

};

#endif