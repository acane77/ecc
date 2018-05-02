    #include "ast/errtoken.h"

namespace Miyuki::AST {

    TokenPtr ErrorReportSupport::getErrorToken() {
        return tok;
    }

    void ErrorReportSupport::setErroToken(const TokenPtr& _tok) {
        tok = _tok;
    }

    TokenPtr ErrorReportSupport::holdToken = nullptr;
    void ErrorReportSupport::holdErrorToken(TokenPtr tok) {
        holdToken = tok;
    }

    ErrorReportSupport::ErrorReportSupport() {
        tok = holdToken;
        holdToken = nullptr;
    }
}