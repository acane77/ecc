#ifndef _MIYUKI_PPCOND_H
#define _MIYUKI_PPCOND_H

// This file defines condition values for Processor

#include "common/ptrdef.h"
#include "lex/token.h"

namespace Miyuki::Parse {

    DEFINE_SHARED_PTR(PreprocCondition)
    typedef deque<PreprocConditionPtr> ConditionHierarchy;

    // preprocessor condition
    class PreprocCondition {
    public:

        bool _condition;
        bool _conditionUsedToBeTrue;
        TokenPtr tok;
        bool parentIsTrue;

        PreprocCondition(bool _condition, const TokenPtr &tok, bool parentIsTrue) : _condition(_condition), tok(tok),
                                                                                    parentIsTrue(parentIsTrue) {}

        const TokenPtr &getTok() const {
            return tok;
        }
        void setTok(const TokenPtr &tok) {
            PreprocCondition::tok = tok;
        }

        virtual bool isTrue() { return _condition; }
        void set(bool b) { _condition = b; _conditionUsedToBeTrue = _conditionUsedToBeTrue || b; }

    };

}

#endif