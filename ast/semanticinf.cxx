#include "ast/semanticinf.h"

namespace Miyuki::AST {
    Identifier::Identifier(const Lex::TokenPtr & _tok, string _name, uint16_t flag)
        :tok(_tok), name(_name) {
        perproty.flag = flag;
    }
}