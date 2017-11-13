#ifndef _MIYUKI_DEFINES_H
#define _MIYUKI_DEFINES_H

// Prefedined macros and constants

#include "common/ptrdef.h"

#define ishexdigit(x) ( isdigit(x) || (x >= 'A' && x <= 'F') || (x >= 'a' && x <= 'f') )
#define isoctdigit(x) ( x >= '0' && x <= '8' )

typedef long double FloatingType;
typedef int64_t     IntegerType;

#endif
