#include "texture.h"

#include <stdlib.h>

typedef struct Texture {
    Image* image;
} Texture;

Texture* Texture_New(void)
{
    Texture* tex = calloc(1, sizeof(*tex));
    return tex;
}

Texture* Texture_Import_BMP(Texture* tex, FILE* fd)
{
    Image_Delete(tex->image);
    tex->image = Image_Import_BMP(fd);

    if (tex->image == NULL) {
        return NULL;
    }

    return tex;
}

Texture* Texture_Import_Color(Texture* tex, Color color)
{
    Image_Delete(tex->image);
    tex->image = Image_New(1, 1);

    RGB rgb = RGB_FromColor(color);
    Image_SetPixel(tex->image, 0, 0, rgb);

    return tex;
}

void Texture_Delete(Texture* tex)
{
    Image_Delete(tex->image);
    free(tex);
}

// TODO: need to convert the image to a Color array for faster compute
// TODO: need to implement some kind of interpolation
Color Texture_ColorAt(const Texture* tex, point2 st)
{
    // might need to round this down with some guarantee
    size_t xx = (size_t)(st.x * (tex->image->res.width - 1));
    size_t yy = (size_t)((1.0f - st.y) * (tex->image->res.height - 1));

    RGB pix = Image_GetPixel(tex->image, xx, yy);
    return Color_FromRGB(pix);
}
