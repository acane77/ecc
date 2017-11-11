#include "symbols.h"

namespace Miyuki::AST {

    bool IEvaluatable::IsCalculated() const {
        return isCalculated;
    }

    void IEvaluatable::setIsCalculated(bool isCalculated) {
        IEvaluatable::isCalculated = isCalculated;
    }

    const TypePtr &IEvaluatable::getSymbolType() const {
        return symbolType;
    }

    void IEvaluatable::setSymbolType(const TypePtr &symbolType) {
        IEvaluatable::symbolType = symbolType;
    }

    const TokenPtr &IEvaluatable::getCalculatedToken() const {
        return calculatedToken;
    }

    void IEvaluatable::setCalculatedToken(const TokenPtr &calculatedToken) {
        IEvaluatable::calculatedToken = calculatedToken;
    }
}