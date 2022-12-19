#pragma once

#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
#    include <type_traits>
#endif

#define static_assert_decl(const_expr) static_assert((const_expr), "Assertion false: " #const_expr)
#ifndef __cplusplus
#    define static_assert_expr(const_expr) (0 * sizeof(struct { static_assert_decl(const_expr); }))
#else
#    define static_assert_expr(const_expr)              \
        (                                               \
            [=]() {                                     \
                static_assert(const_expr, #const_expr); \
            },                                          \
            0)
#endif

#ifndef __cplusplus
#    define same_type(a, b) (__builtin_types_compatible_p(__typeof__(a), __typeof__(b)))
#else
#    define same_type(a, b) (std::is_same_v<decltype(a), decltype(b)>)
#endif
#define is_array(a) static_assert_expr(!same_type((a), &(a)[0]))

#define containerof(ptr, type, member) ((type*)((char*)ptr - offsetof(type, member)))
#define lengthof(arr)                  (is_array(arr), sizeof(arr) / sizeof((arr)[0]))
#define likely(x)                      __builtin_expect(!!(x), 1)
#define unlikely(x)                    __builtin_expect(!!(x), 0)

#define attr_naked              __attribute__((naked))
#define attr_aligned(alignment) __attribute__((packed, aligned(alignment)))

#define global
#define intern       static
#define thread_local __thread
// TODO: should we use this?
// #define inline __attribute__((always_inline))
#define noinline __attribute__((noinline))

#define OPTIMIZE_UNREACHABLE __builtin_unreachable()
#define OPTIMIZE_ASSUME(expr)     \
    do {                          \
        if (!(expr))              \
            OPTIMIZE_UNREACHABLE; \
    } while (0)

#if !defined(NDEBUG)
#    define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#    define DEBUG_PRINT(...)
#endif

// TODO: check what this does in GDB, if it doesn't trigger a breakpoint at the abort call, use int3 or equiv
#define ABORT(msg, ...)                                                                                  \
    do {                                                                                                 \
        fprintf(stderr, "Aborted @ %s:%s:%d :: " msg "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
        abort();                                                                                         \
    } while (0)

// TODO: overload, 2+ args provides msg
#define ASSERT(cond)                                   \
    do {                                               \
        if (!(cond)) {                                 \
            fprintf(                                   \
                stderr,                                \
                "\n\nAssertion failed @ %s:%s:%d ::\n" \
                "%s\n\n",                              \
                __FILE__,                              \
                __func__,                              \
                __LINE__,                              \
                #cond);                                \
            asm volatile("int3");                      \
        }                                              \
    } while (0)

#define MAX(a, b)             \
    ({                        \
        __typeof__(a) a_ = a; \
        __typeof__(b) b_ = b; \
        a_ > b_ ? a_ : b_;    \
    })

#define MIN(c, d)             \
    ({                        \
        __typeof__(c) c_ = c; \
        __typeof__(d) d_ = d; \
        c_ < d_ ? c_ : d_;    \
    })

#define CLAMP(t, e, f)        \
    ({                        \
        __typeof__(t) t_ = t; \
        __typeof__(e) e_ = e; \
        __typeof__(f) f_ = f; \
        MIN(MAX(t_, e_), f_); \
    })
