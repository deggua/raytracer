#pragma once

#include <assert.h>
#include <stdint.h>

#include "math/vec.h"

#define COLOR_RED    ((const Color){.r = 1.0f, .g = 0.0f, .b = 0.0f})
#define COLOR_BLUE   ((const Color){.r = 0.8f, .g = 0.8f, .b = 1.0f})
#define COLOR_GREY   ((const Color){.r = 0.5f, .g = 0.5f, .b = 0.5f})
#define COLOR_WHITE  ((const Color){.r = 1.0f, .g = 1.0f, .b = 1.0f})
#define COLOR_BLACK  ((const Color){.r = 0.0f, .g = 0.0f, .b = 0.0f})
#define COLOR_YELLOW ((const Color){.r = 0.8f, .g = 0.6f, .b = 0.2f})
#define COLOR_GREEN  ((const Color){.r = 0.3f, .g = 0.8f, .b = 0.3f})
#define COLOR_NAVY   ((const Color){.r = 0.1f, .g = 0.2f, .b = 0.5f})

typedef struct {
    u8 b;
    u8 g;
    u8 r;
} RGB;

typedef vec3 Color;

static_assert(sizeof(RGB) == 3, "RGB pixel should be 3 bytes");

RGB RGB_Brighten(RGB rgb, f32 scalar);
RGB RGB_Blend(RGB color1, RGB color2, f32 weight);
RGB RGB_FromColor(Color color);

Color Color_Brighten(Color color, f32 scalar);
Color Color_BrightenBy(Color color1, Color color2);
Color Color_Blend(Color color1, Color color2, f32 weight);
Color Color_FromRGB(RGB rgb);
Color Color_Tint(Color color1, Color color2);
