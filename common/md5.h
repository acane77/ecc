#ifndef _MIYUKI_MD5_H
#define _MIYUKI_MD5_H

#include <cstdlib>
#include <cstring>
#include <cstdint>

namespace Miyuki::Common {
    void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest);
    uint64_t md5(const char *msg);
}

#endif