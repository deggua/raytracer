#pragma once

#include "gfx/image.h"
#include "world/camera.h"
#include "world/scene.h"

typedef struct {
    Camera*   cam;
    Scene*    scene;
    ImageRGB* img;
} RenderCtx;

RenderCtx* Render_New(in Scene* scene, in ImageRGB* img, in Camera* cam);
void       Render_Delete(out RenderCtx* ctx);
ImageRGB*  Render_Do(in RenderCtx* ctx, size_t samples_per_pixel, size_t max_ray_depth, size_t num_threads);
