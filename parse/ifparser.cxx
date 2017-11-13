#include "ifparser.h"
#include "common/console.h"
#include "common/obsevent.h"

namespace Miyuki::Parse {

    IParser * IParser::instance = nullptr;
    deque<ParseError> IParser::errors;
    size_t IParser::errorCount = 0;

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
reget_token:
        if (m_tsptr_r == m_tsptr_w) {
            try {
                look = M_lex->scan();
                tokens[(++m_tsptr_w) % MaxRetractSize] = look;
                m_tsptr_r++;
            }
            catch (SyntaxError& err) {
                // notice: we cannot pass look token, because lexer do not generate
                //   token at all if there's lexical error, so generate a token for
                //   error mamually.
                diagError(err.what(), M_lex->getLexedInvalidToken());
                goto reget_token;
            }
            return look;
        }
        else if (m_tsptr_r < m_tsptr_w) {
            look = tokens[(++m_tsptr_r) % MaxRetractSize];
            return look;
        }
        else assert( false && "stack overflow." );
    }

    TokenPtr IParser::retract() {
        if (m_tsptr_w - m_tsptr_r > MaxRetractSize || m_tsptr_r <= 0)
            assert( false && "Invaild retract operation." );
        look = tokens[(--m_tsptr_r) % MaxRetractSize];
        return look;
    }

    void IParser::reportError(std::ostream& os) {
        for (ParseError& e : errors) {
            TokenPtr tok = e.getToken();
            string s = M_lex->getSourceManager()->getLine(tok->filenam, tok->row);
            for (int i=0 ; i < s.length() ; i++) {
                if (s[i] == '\t') os << "    ";
                else if (s[i] != '\n') os << s[i];
            }
            os << endl;
            // output blank before token
            for (int i=0; i<tok->startCol - 1; i++) {
                if (s[i] == '\t') os <<"    ";
                else os << ' ';
            }
            os << "^";
            for (int i=tok->startCol ; i<tok->column; i++) {
                if (s[i] == '\t') os << "~~~~";
                else os << '~';
            }
            os << endl;
            if (e.isWarning())  os << Miyuki::Console::Warning();
            else os << Miyuki::Console::Error();
            os << tok->filenam << ":" << tok->row << ":" << tok->column << ": " << e.what() << endl << endl;

            // TODO: print include from after include implemented
        }
    }

    void IParser::parseDone() {
        reportError(cout);
        if ( errorCount != 0 )  throw PasreCannotRecoveryException();
    }

    bool IParser::skipUntil(const deque<int32_t>& toks, uint32_t flag) {
        // First we find token we want
        for ( ; ; next() ) {
            for (int32_t expect : toks ) {
                if (look->is(expect)) {
                    if (hasRecoveryFlag(flag, RecoveryFlag::KeepSpecifiedToken));// do nothing
                    else next();
                    return true;
                }
            }

            if ( hasRecoveryFlag(flag, RecoveryFlag::SkipUntilSemi) && look->is(';') )
                return false;

            // meet EOF and is required
            if ( look->is(-1) && toks.size() == 1 && toks[0] == -1)
                return true;

            // token runs out
            if ( look->is(-1) )
                return false;

            // TODO: add_special_skip_rules
        }
    }

    void IParser::notify(uint32_t what) {
        if (what == ObserverEvent::SM_SWITCHING_FILE) {
            cout << Console::Green("   Info  ") << MSG_SM_SWOTCHING_FILE <<"\n";
        }
    }

    void IParser::cacheToken(TokenPtr tok) {
        assert( m_tsptr_r == m_tsptr_w && "read pointer is not at top of stack" );
        tokens[(++m_tsptr_w) % MaxRetractSize] = tok;
        m_tsptr_r++;
    }
}

