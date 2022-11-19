#pragma once

#include <stdint.h>

#include "common/macros.h"

typedef float  f32;
typedef double f64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint_fast8_t  u8_fast;
typedef uint_fast16_t u16_fast;
typedef uint_fast32_t u32_fast;
typedef uint_fast64_t u64_fast;

typedef int_fast8_t  i8_fast;
typedef int_fast16_t i16_fast;
typedef int_fast32_t i32_fast;
typedef int_fast64_t i64_fast;

typedef uint_least8_t  u8_min;
typedef uint_least16_t u16_min;
typedef uint_least32_t u32_min;
typedef uint_least64_t u64_min;

typedef int_least8_t  i8_min;
typedef int_least16_t i16_min;
typedef int_least32_t i32_min;
typedef int_least64_t i64_min;

typedef union {
    f32 f32;

    struct {
        u32 frac : 23;
        u32 exp  : 8;
        u32 sign : 1;
    };

    u32 u32;
    u8  u8[sizeof(f32)];
} f32_ieee754;

typedef union {
    f64 f64;

    struct {
        u64 frac : 52;
        u64 exp  : 11;
        u64 sign : 1;
    };

    u64 u64;
    u8  u8[sizeof(f64)];
} f64_ieee754;

#if !defined(__cplusplus) && !defined(TARGET_WINDOWS)
#    include <stdbool.h>

#    undef bool
#    undef true
#    undef false

typedef _Bool bool;

enum {
    false = 0,
    true  = 1
};
#endif

static_assert_decl(sizeof(f32) == 4);
static_assert_decl(sizeof(f64) == 8);
static_assert_decl(sizeof(f32_ieee754) == 4);
static_assert_decl(sizeof(f64_ieee754) == 8);
