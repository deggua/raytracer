#include "gfx/color.h"

#include "common/math.h"

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
    const f32 weightColor1 = 1.0f - weight;
    const f32 weightColor2 = weight;
    return (RGB){
        .r = weightColor1 * color1.r + weightColor2 * color2.r,
        .g = weightColor1 * color1.g + weightColor2 * color2.g,
        .b = weightColor1 * color1.b + weightColor2 * color2.b,
    };
}

RGB RGB_FromColor(Color color)
{
    return (RGB){
        .r = 255 * color.r,
        .g = 255 * color.g,
        .b = 255 * color.b,
    };
}

Color Color_Brighten(Color color, f32 scalar)
{
    return (Color){
        .vec3 = vmul(color.vec3, scalar),
    };
}

Color Color_Blend(Color color1, Color color2, f32 blend)
{
    return (Color){
        .vec3 = vadd(vmul(color1.vec3, blend), vmul(color2.vec3, 1.0f - blend)),
    };
}

Color Color_FromRGB(RGB rgb)
{
    return (Color){
        .r = rgb.r / 255.0f,
        .g = rgb.g / 255.0f,
        .b = rgb.b / 255.0f,
    };
}

Color Color_Tint(Color color1, Color color2)
{
    return (Color){
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
