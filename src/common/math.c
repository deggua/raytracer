#include <immintrin.h>

#if 1
// Method 1: Simple SSE
//  CP 7
//  LCD 7
//  Vec3 (xyz) = 34.71, 34.60, 35.74
static inline __m128 cross_product(const __m128 vec0, const __m128 vec1)
{
    const __m128 tmp0 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 0, 2, 1)); // 1, 1
    const __m128 tmp1 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2)); // 1, 1
    const __m128 tmp2 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 1, 0, 2)); // 1, 1
    const __m128 tmp3 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1)); // 1, 1
    return _mm_sub_ps(                                                       // 4, 0.5
        _mm_mul_ps(tmp0, tmp1),
        _mm_mul_ps(tmp2, tmp3));
}
#elif 0
// Method 2: Simple SSE (FMA instructions)
//  CP 5
//  LCD 9
//  Vec3 (xyz) = 35.95, 33.94, 35.92
static inline __m128 cross_product(const __m128 vec0, const __m128 vec1)
{
    const __m128 tmp0 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 0, 2, 1));
    const __m128 tmp1 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2));
    const __m128 tmp2 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 1, 0, 2));
    const __m128 tmp3 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1));
    const __m128 tmp4 = _mm_mul_ps(tmp2, tmp3);
    return _mm_fmsub_ps(tmp0, tmp1, tmp4);
}
#elif 0
// Method 3: Fewer swizzles, swizzle after subtraction
//  CP 8
//  LCD 8
//  Vec3 (xyz) = 36.28, 35.75, 35.08
static inline __m128 cross_product(const __m128 vec0, const __m128 vec1)
{
    __m128 tmp0       = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 tmp1       = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 0, 2, 1));
    tmp0              = _mm_mul_ps(tmp0, vec0);
    tmp1              = _mm_mul_ps(tmp1, vec1);
    const __m128 tmp2 = _mm_sub_ps(tmp0, tmp1);
    return _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
}
#elif 0
// Method 4: Fewer swizzles, swizzle after subtraction (FMA instructions)
//  CP 10
//  LCD 10
//  Vec3 (xyz) = 35.04, 35.66, 36.41
static inline __m128 cross_product(const __m128 vec0, const __m128 vec1)
{
    const __m128 tmp0 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1));
    __m128       tmp1 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 0, 2, 1));
    tmp1              = _mm_mul_ps(tmp1, vec1);
    const __m128 tmp2 = _mm_fmsub_ps(tmp0, vec0, tmp1);
    return _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
}
#elif 0
// Method 5: Fewer swizzles, swizzle before subtraction
//  CP 8
//  LCD 7
//  Vec3 (xyz) = 35.09, 36.14, 36.10
static inline __m128 cross_product(const __m128 vec0, const __m128 vec1)
{
    const __m128 tmp0 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 0, 2, 1));
    const __m128 tmp1 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2));
    const __m128 tmp2 = _mm_mul_ps(tmp0, vec1);
    const __m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
    const __m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
    return _mm_sub_ps(tmp3, tmp4);
}
#elif 0
// Method 6: Fewer swizzles, swizzle before subtraction (FMA instructions)
//  CP 10
//  LCD 6
//  Vec3 (xyz) = 35.93, 36.37, 36.12
static inline __m128 cross_product(const __m128 vec0, const __m128 vec1)
{
    const __m128 tmp0 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 0, 2, 1));
    const __m128 tmp1 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2));
    const __m128 tmp2 = _mm_mul_ps(tmp0, vec1);
    const __m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
    return _mm_fmsub_ps(tmp0, tmp1, tmp4);
}
#endif

#if 0
// Vec3 (xyz) = 35.54, 34.91, 36.71
Vec3 Vec3_Cross(Vec3 v1, Vec3 v2)
{
    return (Vec3){
        .x = (v1.y * v2.z - v1.z * v2.y),
        .y = (v1.z * v2.x - v1.x * v2.z),
        .z = (v1.x * v2.y - v1.y * v2.x),
    };
}
#else

union f32x4 {
    __m128 m128;
    float  f32[4];
    struct {
        float x;
        float y;
        float z;
        float w;
    };
};

Vec3 Vec3_Cross(Vec3 v1, Vec3 v2)
{
    const __m128      v1ps = _mm_set_ps(0, v1.z, v1.y, v1.x);
    const __m128      v2ps = _mm_set_ps(0, v2.z, v2.y, v2.x);
    const __m128      vcps = cross_product(v1ps, v2ps);
    const union f32x4 cp   = {.m128 = vcps};

    return (Vec3){
        .x = cp.f32[0],
        .y = cp.f32[1],
        .z = cp.f32[2],
    };
}
#endif
