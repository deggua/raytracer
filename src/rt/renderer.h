#pragma once

#include "gfx/image.h"
#include "world/camera.h"
#include "world/scene.h"

typedef struct {
    Camera*   cam;
    Scene*    scene;
    ImageRGB* img;
} RenderCtx;

RenderCtx* Render_New(Scene* scene, ImageRGB* img, Camera* cam);
void       Render_Delete(RenderCtx* ctx);
ImageRGB*  Render_Do(RenderCtx* ctx, size_t samples_per_pixel, size_t max_ray_depth, size_t num_threads);
