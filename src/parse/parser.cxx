#include "parser.h"

namespace Miyuki::Parse {

    Parser::Parser(const char * path) {
        M_proc = make_shared<Preprocessor>(path);
        assert (M_proc && "cannot create preprocessor");

        // get lexer of preprocessor. 
        //   NOTE: we do not mean get lexer to lexing source, lexer is use for report error
        //         and retrive source manager.
        M_lex = M_proc->getPreprocessorLexer();

        M_proc->prepareGetToken(); // initial preprocessor
        next(); // get first token
    }

    TokenPtr Parser::next() {
    reget_token:
        if (m_tsptr_r == m_tsptr_w) {
            try {
                look = M_proc->nextToken();
                tokens[(++m_tsptr_w) % MaxRetractSize] = look;
                m_tsptr_r++;
            }
            catch (SyntaxError& err) {
                // notice: here we pass look because no lexical error will happen
                // TODO: consider if catch block can be removed, because no exception may cause
                diagError(err.what(), look);
                goto reget_token;
            }
            return look;
        }
        else if (m_tsptr_r < m_tsptr_w) {
            look = tokens[(++m_tsptr_r) % MaxRetractSize];
            return look;
        }
        else assert(false && "stack overflow.");
    }

    void Parser::parse() {
        /*TokenSequencePtr toks = getPreprocessedSource();
        for (auto tok : *toks) {
            cout << tok->toSourceLiteral() << " ";
        }

        M_proc->finish(); // finish preprocess*/
        try {
            translationUnit();
        }
        catch (exception& e) {
            cout << "Exception occurred: " << e.what();
        }
        parseDone();
    }

    TokenSequencePtr Parser::getPreprocessedSource() {
        TokenSequencePtr toks = make_shared<TokenSequence>();
        while (look->isNot(Tag::EndOfFile)) {
            toks->push_back(look);
            next();
        }
        return toks;
    }

}