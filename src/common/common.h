#pragma once

#define static_assert_decl(const_expr) _Static_assert(const_expr, "Assertion false: " #const_expr)
#define static_assert_expr(const_expr) (0 * sizeof(struct { static_assert_decl(const_expr); }))

#define same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define is_array(a)     static_assert_expr(!same_type((a), &(a)[0]))

#define containerof(ptr, type, member) ((type*)((char*)ptr - offsetof(type, member)))
#define lengthof(array)                (sizeof(array) / sizeof((array)[0]) + is_array(array))
#define likely(x)                      __builtin_expect(!!(x), 1)
#define unlikely(x)                    __builtin_expect(!!(x), 0)

#define attr_naked              __attribute__((naked))
#define attr_aligned(alignment) __attribute__((packed, aligned(alignment)))

#define OPTIMIZE_UNREACHABLE __builtin_unreachable()
#define OPTIMIZE_ASSUME(expr)     \
    do {                          \
        if (!(expr))              \
            OPTIMIZE_UNREACHABLE; \
    } while (0)

#include "common/types.h"
