#ifndef _MIYUKI_SEMANTICINF_H
#define _MIYUKI_SEMANTICINF_H

#include "lex/token.h"
#include "ast/type.h"

// this file defines semantic-analyse-time information, such as Identifier types

namespace Miyuki::AST {
	
    DEFINE_SHARED_PTR(Identifier)

    // Identifier information
    class Identifier {
    public:
        // initial-defination token.
        //   use for error report etc.
        Lex::TokenPtr  tok;

        // name of identifier
        string         name;

        // type of identifier
        TypePtr        type;

        union {
            struct {
                // storage-class
                bool           isStatic : 1;
                bool           isRegister : 1;
                bool           isExtern : 1;
                bool           isThreadLocal : 1;
                bool           isAuto : 1;

                // qualifier
                bool           isConst : 1;
                bool           isRestrict : 1;
                bool           isVolatile : 1;

                // unused
                bool          : 8;
            } values;
            uint16_t flag;
        }               perproty;

        enum {
            FLAG_IS_STATIC = 1 << 15, 
            FLAG_IS_REGISTER = 1 << 14,
            FLAG_IS_EXTERN = 1 << 13,
            FLAG_IS_THREAD_LOCAL = 1 << 12,
            FLAG_IS_AUTO = 1 << 11,
            FLAG_IS_CONST = 1 << 10,
            FLAG_IS_RESTRICT = 1 << 9,
            FLAG_IS_VOLATILE = 1 << 8
        };
      
        Identifier(const Lex::TokenPtr& _tok, string _name, uint16_t flag);
    };
}

#endif