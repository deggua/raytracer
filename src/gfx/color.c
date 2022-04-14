#include "gfx/color.h"

#include "gfx/utils.h"

RGB RGB_Brighten(RGB rgb, float scalar) {
    return (RGB){
        .r = clampf(rgb.r * scalar, 0, 255),
        .g = clampf(rgb.g * scalar, 0, 255),
        .b = clampf(rgb.b * scalar, 0, 255),
    };
}

RGB RGB_Blend(RGB color1, RGB color2, float weight) {
    const float weightColor1 = 1.0f - weight;
    const float weightColor2 = weight;
    return (RGB){
        .r = weightColor1 * color1.r + weightColor2 * color2.r,
        .g = weightColor1 * color1.g + weightColor2 * color2.g,
        .b = weightColor1 * color1.b + weightColor2 * color2.b,
    };
}

RGB RGB_FromFloat(float red, float green, float blue) {
    return (RGB){
        .r = 255 * red,
        .g = 255 * green,
        .b = 255 * blue,
    };
}

Color Color_Brighten(Color color, float scalar) {
    return vmul(color, scalar);
}

Color Color_Blend(Color color1, Color color2, float weight) {
    return vadd(vmul(color1, 1.0f - weight), vmul(color2, weight));
}

Color Color_FromRGB(RGB rgb) {
    return (Color){
        .x = rgb.r / 255.0f,
        .y = rgb.g / 255.0f,
        .z = rgb.b / 255.0f,
    };
}

Color Color_Tint(Color color1, Color color2) {
    return (Color){
        .x = color1.x * color2.x,
        .y = color1.y * color2.y,
        .z = color1.z * color2.z,
    };
}
