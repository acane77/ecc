#ifndef _MIYUKI_ERR_TOKEN_H
#define _MIYUKI_ERR_TOKEN_H

#include "lex/token.h"

namespace Miyuki::AST {

    using namespace Lex;

    class ErrorReportSupport {
    public:
        TokenPtr  tok = nullptr;
        
        TokenPtr getErrorToken();
        void setErroToken(const TokenPtr& _tok);

        // global value, provide token hold support, directly use it when wang to set err_token for
        // a symbol
        static TokenPtr holdToken;
        static void holdErrorToken(TokenPtr tok);

        ErrorReportSupport();
    };

}

#endif