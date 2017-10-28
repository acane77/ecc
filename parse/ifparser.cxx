#include "ifparser.h"

namespace Miyuki::Parse {

    void IParser::match(uint32_t term, string &&errmsg, TokenPtr &ptr) {
        if (look->isNot(term)) {
            diagError(std::move(errmsg), ptr);
            if (look->is(-1))  parseDone();
            if (!skipUntil({ ';' }, RecoveryFlag::KeepSpecifiedToken | RecoveryFlag::SkipUntilSemi))
                parseDone();
            return;
        }
        next();
    }

    TokenPtr IParser::next() {
        if (m_tsptr_r == m_tsptr_w) {
            look = M_lex->scan();
            tokens[(++m_tsptr_w) % MaxRetractSize] = look;
            m_tsptr_r++;
            //DEBUG_LEXER_PARSER(printf("Fetching from lexer: tok->tag=%s\n",  m_look->toString().c_str());)
        }
        else if (m_tsptr_r < m_tsptr_w) {
            look = tokens[(++m_tsptr_r) % MaxRetractSize];
            //DEBUG_LEXER_PARSER(printf("Fetching from retract: tok->tag=%s\n",  m_look->toString().c_str());)
        }
        else assert( false && "stack overflow." );
    }

    TokenPtr IParser::retract() {
        if (m_tsptr_w - m_tsptr_r > MaxRetractSize || m_tsptr_r <= 0)
            assert( false && "Invaild retract operation." );
        look = tokens[(--m_tsptr_r) % MaxRetractSize];
    }

    void IParser::reportError(std::ostream& os) {
        for (ParseError& e : errors) {
            TokenPtr tok = e.getToken();
            string s = M_lex->getSourceManager()->getLine(tok->filenam, tok->row);
            int tabcount = 0;
            for (int i=0; i<s.length(); i++)
                if (s[i] == '\t') { cout << "\t"; tabcount++; }
            for (int i=1; i<Miyuki::Lex::Token::startColumn; i++)
                os << " ";
            os << "^";
            for (int i=Miyuki::Lex::Token::startColumn+1; i<M_lex->getSourceManager()->getColumn(); i++)
                os << "~";
            os << endl << ( e.isWarning() ? "warning" : "error" ) << ": " << e.what();
        }
    }

    void IParser::parseDone() {
        reportError(cout);
        if ( errors.size() != 0 )  throw PasreCannotRecoveryException();
    }
}