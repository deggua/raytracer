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

void    Skybox_Delete(inout Skybox* skybox);
Skybox* Skybox_Import_BMP(const char* folder);
Color   Skybox_ColorAt(in Skybox* skybox, vec3 dir);
