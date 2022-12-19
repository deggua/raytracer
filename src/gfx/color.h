#pragma once

#include <assert.h>
#include <stdint.h>

#include "math/vec.h"

// WARNING: don't change these
#define COLOR_WHITE ((Color){.r = 1.0f, .g = 1.0f, .b = 1.0f})
#define COLOR_BLACK ((Color){.r = 0.0f, .g = 0.0f, .b = 0.0f})
#define COLOR_RED   ((Color){.r = 1.0f, .g = 0.0f, .b = 0.0f})
#define COLOR_GREEN ((Color){.r = 0.0f, .g = 1.0f, .b = 0.0f})
#define COLOR_BLUE  ((Color){.r = 0.0f, .g = 0.0f, .b = 1.0f})

#define COLOR_GREY   ((Color){.r = 0.5f, .g = 0.5f, .b = 0.5f})
#define COLOR_YELLOW ((Color){.r = 0.8f, .g = 0.6f, .b = 0.2f})
#define COLOR_NAVY   ((Color){.r = 0.1f, .g = 0.2f, .b = 0.5f})

typedef struct {
    u8 b;
    u8 g;
    u8 r;
} RGB;

typedef vec3 Color;

static_assert_decl(sizeof(RGB) == 3);

RGB RGB_Brighten(RGB rgb, f32 scalar);
RGB RGB_Blend(RGB color1, RGB color2, f32 weight);
RGB RGB_FromColor(Color color);

Color Color_Brighten(Color color, f32 scalar);
Color Color_BrightenBy(Color color1, Color color2);
Color Color_Blend(Color color1, Color color2, f32 weight);
Color Color_FromRGB(RGB rgb);
Color Color_Tint(Color color1, Color color2);
f32   Color_Luminance(Color color);
