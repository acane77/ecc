#ifndef _MIYUKI_OBSEVENT_H
#define _MIYUKI_OBSEVENT_H

#include <cstdint>

// this file define observer event
namespace Miyuki::Common::ObserverEvent {
    enum : uint32_t {
        // for source mamager
            SM_SWITCHING_FILE = 100
#define MSG_SM_SWOTCHING_FILE "File swiched"
    };
};

#endif