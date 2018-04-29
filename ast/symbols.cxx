#include "symbols.h"

namespace Miyuki::AST {

    using namespace Miyuki::Lex;

    bool Symbol::isPreprocessorSymbol = false;

    bool IEvaluatable::IsCalculated() const {
        return isCalculated;
    }

    void IEvaluatable::setIsCalculated(bool isCalculated) {
        IEvaluatable::isCalculated = isCalculated;
    }

    const TokenPtr &IEvaluatable::getCalculatedToken() const {
        return calculatedToken;
    }

    void IEvaluatable::setCalculatedToken(const TokenPtr &calculatedToken) {
        setIsCalculated(true);
        IEvaluatable::calculatedToken = calculatedToken;
    }

    void IEvaluatable::copyEvalPerproty(IEvaluatablePtr another) {
        assert(another && "another is nullptr");
        setCalculatedToken( another->getCalculatedToken() );
        setIsCalculated( another->isCalculated );
    }

    TypePtr Miyuki::AST::IEvaluatable::getSymbolType() {
        return symbolType;
    }

    void Miyuki::AST::IEvaluatable::setSymbolType(TypePtr type) {
        symbolType = type;
    }

    void Miyuki::AST::IEvaluatable::setAddr(Value * _addr, bool isLV) {
        addr = _addr;
        isLValue = isLV;
        symbolType = _addr->getType();
    }

    void Miyuki::AST::IEvaluatable::lvalue(Value * addr) {
        setAddr(addr, true);
    }

    void Miyuki::AST::IEvaluatable::rvalue(Value * addr) {
        setAddr(addr, false);
    }
}