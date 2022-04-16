#pragma once

#include <stdint.h>

#include "common/common.h"

void RNG_Seed(u64 seed);
f32  RNG_Random(void);
f32  RNG_RandomInRange(f32 min, f32 max);
