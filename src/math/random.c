#include "random.h"

#include <immintrin.h>
#include <stddef.h>
#include <stdint.h>

/* This is xoshiro128+ 1.0, our best and fastest 32-bit generator for 32-bit
   f32ing-point numbers. We suggest to use its upper bits for
   f32ing-point generation, as it is slightly faster than xoshiro128**.
   It passes all tests we are aware of except for
   linearity tests, as the lowest four bits have low linear complexity, so
   if low linear complexity is not considered an issue (as it is usually
   the case) it can be used to generate 32-bit outputs, too.

   We suggest to use a sign test to extract a random Boolean value, and
   right shifts to extract subsets of bits.

   The state must be seeded so that it is not everywhere zero. */

static inline u32 rotl(const u32 x, int k)
{
    return (x << k) | (x >> (32 - k));
}

static __thread u32 s[4];

static u32 next(void)
{
    const u32 result = s[0] + s[3];

    const u32 t = s[1] << 9;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 11);

    return result;
}

/* This is the jump function for the generator. It is equivalent
   to 2^64 calls to next(); it can be used to generate 2^64
   non-overlapping subsequences for parallel computations. */

static void jump(void)
{
    static const u32 JUMP[] = {0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b};

    u32 s0 = 0;
    u32 s1 = 0;
    u32 s2 = 0;
    u32 s3 = 0;
    for (size_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
        for (size_t b = 0; b < 32; b++) {
            if (JUMP[i] & UINT32_C(1) << b) {
                s0 ^= s[0];
                s1 ^= s[1];
                s2 ^= s[2];
                s3 ^= s[3];
            }
            next();
        }

    s[0] = s0;
    s[1] = s1;
    s[2] = s2;
    s[3] = s3;
}

void Random_Seed(u64 s1, u64 s2)
{
    s[0] = (u32)s1;
    s[1] = (u32)(s1 >> 32);
    s[2] = (u32)s2;
    s[3] = (u32)(s2 >> 32);
    jump();
}

f32 Random_Normal_f32(void)
{
    u32         nextVal = next();
    f32_ieee754 conv    = {.u32 = (nextVal >> 9) | 0x3f800000};
    return conv.f32 - 1.0f;
}

f32 Random_Range_f32(f32 min, f32 max)
{
    return min + (max - min) * Random_Normal_f32();
}

void Random_Seed_HighEntropy(void)
{
    unsigned long long s1, s2;
    while (!_rdrand64_step(&s1)) {}
    while (!_rdrand64_step(&s2)) {}

    Random_Seed((u64)s1, (u64)s2);
}
