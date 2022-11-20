#pragma once

#include "gfx/color.h"
#include "gfx/texture.h"
#include "math/vec.h"

typedef struct {
    union {
        struct {
            Texture *xpos, *xneg;
            Texture *ypos, *yneg;
            Texture *zpos, *zneg;
        };

        Texture* tex[6];
    };
} Skybox;

void    Skybox_Delete(Skybox* skybox);
Skybox* Skybox_Import_BMP(const char* folder_path);
Color   Skybox_ColorAt(Skybox* skybox, vec3 dir);
