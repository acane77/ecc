#include "ast/value.h"
#include "ast/env.h"
#include "llvm/IR/Constants.h"

namespace Miyuki::AST {

    bool ValueFlag::isConditionExpression = false;

    Value * Miyuki::AST::ValueFactory::build(TokenPtr tok) {
        LLVMContext& ctx = GlobalScope::getInstance().context;
        if (tok->is(Tag::Integer)) {
            IntTokenPtr itok = static_pointer_cast<IntToken>(tok);
            if (itok->bit == 64)
                return ConstantInt::getIntegerValue(Type::getInt64Ty(ctx), APInt(64, itok->value, itok->isSigned));
            return ConstantInt::getIntegerValue(Type::getInt32Ty(ctx), APInt(32, itok->value, itok->isSigned));
        }
        if (tok->is(Tag::Floating)) {
            FloatTokenPtr ftok = static_pointer_cast<FloatToken>(tok);
            if (ftok->bit == 64)
                return ConstantFP::get(ctx, APFloat((double)ftok->value));
            return ConstantFP::get(ctx, APFloat((float)ftok->value));
        }
        if (tok->is(Tag::Character)) {
            CharTokenPtr ctok = static_pointer_cast<CharToken>(tok);
            return ConstantInt::getIntegerValue(Type::getInt8Ty(ctx), APInt(8, ctok->value, false));
        }
        if (tok->is(Tag::StringLiteral)) {
            StringTokenPtr stok = static_pointer_cast<StringToken>(tok);
            return ConstantDataArray::getString(ctx, stok->value);
        }
        assert(false && "Is not a constant token");
        return nullptr;
    }

}