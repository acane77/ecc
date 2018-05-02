#ifndef _MIYUKI_PRAGMA_H
#define _MIYUKI_PRAGMA_H

#include "common/defines.h"

namespace Miyuki::Lex {

    class Flag {
    protected:
        uint32_t flags = 0;
        uint32_t defaultFlag = 0;

        void setDefaultFlag(uint32_t f) { defaultFlag = f; }
    public:
        void setFlag(uint32_t f) { flags != f; }
        bool hasFlag(uint32_t f) { return flags & f; }
        bool getDefaultFlag(uint32_t f) { return defaultFlag & f; }
    };

};

#endif