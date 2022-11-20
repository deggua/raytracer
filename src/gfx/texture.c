#include "texture.h"

#include <stdlib.h>

typedef struct Texture {
    ImageColor* image;
} Texture;

Texture* Texture_New(void)
{
    Texture* tex = calloc(1, sizeof(*tex));
    return tex;
}

bool Texture_Import_BMP(Texture* tex, FILE* fd)
{
    // create the texture image on the heap
    ImageColor* tmp_tex_img = calloc(1, sizeof(ImageColor));
    if (tmp_tex_img == NULL) {
        goto error_Return;
    }

    // load the BMP from the file into an sRGB image
    ImageRGB tmp_bmp_img;
    if (!ImageRGB_Load_BMP(&tmp_bmp_img, fd)) {
        goto error_CleanupAlloc;
    }

    // convert from sRGB image to linear image
    if (!ImageColor_Load_ImageRGB(tmp_tex_img, &tmp_bmp_img)) {
        goto error_CleanupBMP;
    }

    // unload the sRGB image
    ImageRGB_Unload(&tmp_bmp_img);

    // update tex
    tex->image = tmp_tex_img;
    return true;

error_CleanupBMP:
    ImageRGB_Unload(&tmp_bmp_img);
error_CleanupAlloc:
    free(tmp_tex_img);
error_Return:
    return false;
}

bool Texture_Import_Color(Texture* tex, Color color)
{
    // create the texture image on the heap
    ImageColor* tmp_tex_img = calloc(1, sizeof(ImageColor));
    if (tmp_tex_img == NULL) {
        goto error_Return;
    }

    if (!ImageColor_Load_Empty(tmp_tex_img, 1, 1)) {
        goto error_CleanupAlloc;
    }

    ImageColor_SetPixel(tmp_tex_img, 0, 0, color);

    tex->image = tmp_tex_img;
    return true;

error_CleanupAlloc:
    free(tmp_tex_img);
error_Return:
    return false;
}

void Texture_Unload(Texture* tex)
{
    if (tex->image) {
        ImageColor_Unload(tex->image);
        free(tex->image);
        tex->image = NULL;
    }
}

void Texture_Delete(Texture* tex)
{
    Texture_Unload(tex);
    free(tex);
}

// TODO: need to implement some better interpolation
Color Texture_ColorAt(Texture* tex, point2 st)
{
    // might need to round this down with some guarantee
    size_t xx = (size_t)(st.x * (tex->image->res.width - 1));
    size_t yy = (size_t)((1.0f - st.y) * (tex->image->res.height - 1));

    return ImageColor_GetPixel(tex->image, xx, yy);
}
