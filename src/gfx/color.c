#include "color.h"

#include <math.h>

#include "math/math.h"

intern inline f32 sRGBToLinear(u8 channel)
{
    f32 srgb = channel / 255.0f;

    if (srgb < 0.04045f) {
        return srgb / 12.92f;
    } else {
        return powf((srgb + 0.055f) / 1.055f, 2.4f);
    }
}

intern inline u8 LinearTosRGB(f32 channel)
{
    f32 linear = clampf(channel, 0.0f, 1.0f);

    if (linear < 0.0031308f) {
        f32 srgb = linear * 12.92f;
        return (u8)(255.0f * srgb);
    } else {
        f32 srgb = 1.055f * powf(linear, 1.0f / 2.4f) - 0.055f;
        return (u8)(255.0f * srgb);
    }
}

RGB RGB_Brighten(RGB rgb, f32 scalar)
{
    return (RGB){
        .r = clampf(rgb.r * scalar, 0, 255),
        .g = clampf(rgb.g * scalar, 0, 255),
        .b = clampf(rgb.b * scalar, 0, 255),
    };
}

RGB RGB_Blend(RGB color1, RGB color2, f32 weight)
{
    return (RGB){
        .r = color1.r + weight * (color2.r - color1.r),
        .g = color1.g + weight * (color2.g - color1.g),
        .b = color1.b + weight * (color2.b - color1.b),
    };
}

RGB RGB_FromColor(Color color)
{
    return (RGB){
        .r = LinearTosRGB(color.r),
        .g = LinearTosRGB(color.g),
        .b = LinearTosRGB(color.b),
    };
}

Color Color_Brighten(Color color, f32 scalar)
{
    return vmul(color, scalar);
}

Color Color_BrightenBy(Color color1, Color color2)
{
    return vadd(color1, color2);
}

Color Color_Blend(Color color1, Color color2, f32 blend)
{
    return vadd(color1, vmul(blend, vsub(color2, color1)));
}

Color Color_FromRGB(RGB rgb)
{
    return (Color){
        .r = sRGBToLinear(rgb.r),
        .g = sRGBToLinear(rgb.g),
        .b = sRGBToLinear(rgb.b),
    };
}

Color Color_Tint(Color color1, Color color2)
{
    return vmul(color1, color2);
}

Color Color_Desaturate(Color color, f32 desaturation)
{
    static const Color luminance = {
        .r = 0.299f,
        .g = 0.587f,
        .b = 0.114f,
    };

    Color greyscale = {
        .r = vdot(color, luminance),
        .g = vdot(color, luminance),
        .b = vdot(color, luminance),
    };

    return Color_Blend(color, greyscale, desaturation);
}
