#pragma once

#include <stdint.h>

#include "gfx/primitives.h"

#define COLOR_RED    ((Color){0.9f, 0.3f, 0.3f})
#define COLOR_BLUE   ((Color){0.8f, 0.8f, 1.0f})
#define COLOR_GREY   ((Color){0.5f, 0.5f, 0.5f})
#define COLOR_WHITE  ((Color){1.0f, 1.0f, 1.0f})
#define COLOR_BLACK  ((Color){0.0f, 0.0f, 0.0f})
#define COLOR_YELLOW ((Color){0.8f, 0.6f, 0.4f})
#define COLOR_GREEN  ((Color){0.1f, 0.8f, 0.0f})
#define COLOR_NAVY   ((Color){0.1f, 0.2f, 0.5f})

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
