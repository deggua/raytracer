#include "gfx/color.h"

#include "common/math.h"

RGB RGB_Brighten(RGB rgb, f32 scalar)
{
    return (RGB) {
        .r = clampf(rgb.r * scalar, 0, 255),
        .g = clampf(rgb.g * scalar, 0, 255),
        .b = clampf(rgb.b * scalar, 0, 255),
    };
}

RGB RGB_Blend(RGB color1, RGB color2, f32 weight)
{
    return (RGB) {
        .r = color1.r + weight * (color2.r - color1.r),
        .g = color1.g + weight * (color2.g - color1.g),
        .b = color1.b + weight * (color2.b - color1.b),
    };
}

RGB RGB_FromColor(Color color)
{
    return (RGB) {
        .r = 255 * color.r,
        .g = 255 * color.g,
        .b = 255 * color.b,
    };
}

Color Color_Brighten(Color color, f32 scalar)
{
    return (Color) {
        .vec3 = vmul(color.vec3, scalar),
    };
}

Color Color_BrightenBy(Color color1, Color color2)
{
    return (Color) {
        .vec3 = vadd(color1.vec3, color2.vec3),
    };
}

Color Color_Blend(Color color1, Color color2, f32 blend)
{
    return (Color) {
        .vec3 = vadd(color1.vec3, vmul(blend, vsub(color2.vec3, color1.vec3))),
    };
}

Color Color_FromRGB(RGB rgb)
{
    return (Color) {
        .r = rgb.r / 255.0f,
        .g = rgb.g / 255.0f,
        .b = rgb.b / 255.0f,
    };
}

Color Color_Tint(Color color1, Color color2)
{
    return (Color) {
        .vec3 = vmul(color1.vec3, color2.vec3),
    };
}

Color Color_Desaturate(Color color, f32 desaturation)
{
    const Color luminance = {
        .r = 0.299f,
        .g = 0.587f,
        .b = 0.114f,
    };

    Color greyscale = {
        .r = vdot(color.vec3, luminance.vec3),
        .g = vdot(color.vec3, luminance.vec3),
        .b = vdot(color.vec3, luminance.vec3),
    };

    return Color_Blend(color, greyscale, desaturation);
}
