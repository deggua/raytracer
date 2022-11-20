#include "skybox.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gfx/texture.h"
#include "math/math.h"
#include "math/vec.h"

void Skybox_Delete(Skybox* skybox)
{
    for (size_t ii = 0; ii < lengthof(skybox->tex); ii++) {
        if (skybox->tex[ii] != NULL) {
            Texture_Delete(skybox->tex[ii]);
        }
    }
}

Skybox* Skybox_Import_BMP(const char* folder)
{
    Skybox* skybox = calloc(1, sizeof(*skybox));
    if (skybox == NULL) {
        return NULL;
    }

    char path[256] = {0};

    static const char* filenames[] = {"xpos.bmp", "xneg.bmp", "ypos.bmp", "yneg.bmp", "zpos.bmp", "zneg.bmp"};

    for (size_t ii = 0; ii < lengthof(filenames); ii++) {
        strcat(path, folder);
        strcat(path, "/");
        strcat(path, filenames[ii]);

        FILE* fd = fopen(path, "rb+");
        if (fd == NULL) {
            goto error_load;
        }

        Texture* tex = Texture_New();
        if (tex == NULL) {
            fclose(fd);
            goto error_load;
        }

        if (!Texture_Import_BMP(tex, fd)) {
            fclose(fd);
            goto error_load;
        }

        skybox->tex[ii] = tex;

        fclose(fd);
        memset(path, 0x00, sizeof(path));
    }

    return skybox;

error_load:
    Skybox_Delete(skybox);
    return NULL;
}

Color Skybox_ColorAt(const Skybox* skybox, vec3 dir)
{
    f32 absX = fabsf(dir.x);
    f32 absY = fabsf(dir.y);
    f32 absZ = fabsf(dir.z);

    bool isXPositive = dir.x > 0 ? true : false;
    bool isYPositive = dir.y > 0 ? true : false;
    bool isZPositive = dir.z > 0 ? true : false;

    f32 maxAxis, uc, vc;

    size_t index = 0;

    if (isXPositive && absX >= absY && absX >= absZ) {
        // POSITIVE X
        // u (0 to 1) goes from +z to -z
        // v (0 to 1) goes from -y to +y
        maxAxis = absX;
        uc      = dir.y;
        vc      = dir.z;
        index   = 0;
    } else if (!isXPositive && absX >= absY && absX >= absZ) {
        // NEGATIVE X
        // u (0 to 1) goes from -z to +z
        // v (0 to 1) goes from -y to +y
        maxAxis = absX;
        uc      = -dir.y;
        vc      = dir.z;
        index   = 1;
    } else if (isYPositive && absY >= absX && absY >= absZ) {
        // POSITIVE Y
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from +z to -z
        maxAxis = absY;
        uc      = -dir.x;
        vc      = dir.z;
        index   = 2;
    } else if (!isYPositive && absY >= absX && absY >= absZ) {
        // NEGATIVE Y
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -z to +z
        maxAxis = absY;
        uc      = dir.x;
        vc      = dir.z;
        index   = 3;
    } else if (isZPositive && absZ >= absX && absZ >= absY) {
        // POSITIVE Z
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -y to +y
        maxAxis = absZ;
        uc      = -dir.x;
        vc      = -dir.y;
        index   = 4;
    } else {
        // NEGATIVE Z
        // u (0 to 1) goes from +x to -x
        // v (0 to 1) goes from -y to +y
        maxAxis = absZ;
        uc      = -dir.x;
        vc      = dir.y;
        index   = 5;
    }

    // Convert range from -1 to 1 to 0 to 1
    vec2 uv = (vec2){0.5f * (uc / maxAxis + 1.0f), 0.5f * (vc / maxAxis + 1.0f)};

    return Texture_ColorAt(skybox->tex[index], uv);
}
