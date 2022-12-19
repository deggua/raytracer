#pragma once

#include "pp_magic.h"

#define CTL_OVERLOADABLE __attribute__((overloadable))

#define CTL_NEXT_POW2(x)                                                       \
    ({                                                                         \
        __typeof__(x) _x = (x);                                                \
        _x ? (1ull << ((8 * sizeof(unsigned long)) - __builtin_clzl(_x))) : 1; \
    })

#define CTL_MAX(x, y)         \
    ({                        \
        __typeof__(x) _x = x; \
        __typeof__(y) _y = y; \
        _x > _y ? _x : _y;    \
    })

#define CTL_MIN(x, y)         \
    ({                        \
        __typeof__(x) _x = x; \
        __typeof__(y) _y = y; \
        _x < _y ? _x : _y;    \
    })
