#pragma once

#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>

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

#define global
#define intern static
#define thread_local __thread
// TODO: should we use this?
// #define inline __attribute__((always_inline))
#define noinline __attribute__((noinline))

// TODO: check what this does in GDB, if it doesn't trigger a breakpoint at the abort call, use int3 or equiv
#define ABORT(msg, ...)                                                                                  \
    do {                                                                                                 \
        fprintf(stderr, "Aborted @ %s:%s:%d :: " msg "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
        abort();                                                                                         \
    } while (0)

#define OPTIMIZE_UNREACHABLE __builtin_unreachable()
#define OPTIMIZE_ASSUME(expr)     \
    do {                          \
        if (!(expr))              \
            OPTIMIZE_UNREACHABLE; \
    } while (0)
