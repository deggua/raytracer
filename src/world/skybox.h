#pragma once

#include "common/vec.h"
#include "gfx/color.h"
#include "gfx/texture.h"

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
Skybox* Skybox_Import_BMP(const char* folder);
Color   Skybox_ColorAt(const Skybox* skybox, vec3 dir);
