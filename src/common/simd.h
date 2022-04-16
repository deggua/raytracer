#pragma once

#include <immintrin.h>

#include "common/common.h"
#include "common/types.h"

typedef __m128 m128;
typedef __m256 m256;

typedef union {
    m128 m128;

    f64 f64[2];
    f32 f32[4];

    u64 u64[2];
    u32 u32[4];
    u16 u16[8];
    u8  u8[16];
} v128;

typedef union {
    m256 m256;
    m128 m128[2];

    f64 f64[4];
    f32 f32[8];

    u64 u64[4];
    u32 u32[8];
    u16 u16[16];
    u8  u8[32];
} v256;

typedef union {
    m128 m128;
    f32  f32[4];
} f32x4;

typedef union {
    m256 m256;
    m128 m128[2];
    f32  f32[8];
} f32x8;

typedef union {
    m128 m128;
    f64  f64[2];
} f64x2;

typedef union {
    m256 m256;
    m128 m128;
    f64  f64[4];
} f64x4;

static_assert_decl(sizeof(v128) == 16);
static_assert_decl(sizeof(v256) == 32);
static_assert_decl(sizeof(f32x4) == 16);
static_assert_decl(sizeof(f32x8) == 32);
static_assert_decl(sizeof(f64x2) == 16);
static_assert_decl(sizeof(f64x4) == 32);
