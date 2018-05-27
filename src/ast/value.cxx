#include "ast/value.h"
#include "ast/env.h"
#include "llvm/IR/Constants.h"

namespace Miyuki::AST {

    bool ValueFlag::isConditionExpression = false;

    Value * Miyuki::AST::ValueFactory::build(TokenPtr tok) {
		return TypeUtil::createConstant(tok);
    }

}