#pragma once

#include "gfx/image.h"
#include "platform/threads.h"
#include "world/camera.h"
#include "world/scene.h"

typedef struct {
    Camera*   cam;
    Scene*    scene;
    ImageRGB* img;

    Thread* waiter_thread;
    void*   waiter_args;

    bool finished;
} RenderCtx;

RenderCtx* Render_New(Scene* scene, ImageRGB* img, Camera* cam);
void       Render_Delete(RenderCtx* ctx);
void       Render_Start(RenderCtx* ctx, size_t samples_per_pixel, size_t max_ray_depth);
bool       Render_Done(RenderCtx* ctx);
