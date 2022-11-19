#pragma once

#include <stdint.h>

void Random_Seed(u64 s1, u64 s2);
void Random_Seed_HighEntropy(void);

f32 Random_Normal_f32(void);
f32 Random_Range_f32(f32 min, f32 max);
