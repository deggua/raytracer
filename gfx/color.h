#pragma once

#include <stdint.h>
#include "gfx/primitives.h"

typedef struct {
    uint_fast8_t r;
    uint_fast8_t g;
    uint_fast8_t b;
} RGB;

typedef Vec3 Color;

RGB RGB_Brighten(RGB rgb, float scalar);
RGB RGB_Blend(RGB color1, RGB color2, float weight);
RGB RGB_FromFloat(float red, float green, float blue);

Color Color_Brighten(Color color, float scalar);
Color Color_Blend(Color color1, Color color2, float weight);
Color Color_FromRGB(RGB rgb);
Color Color_Tint(Color color1, Color color2);
