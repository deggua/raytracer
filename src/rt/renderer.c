#include "renderer.h"

#include <math.h>
#include <stdlib.h>

#include "math/math.h"
#include "math/random.h"
#include "platform/threads.h"

typedef struct {
    size_t w, h;
    size_t x, y;
} Tile;

typedef union {
    u8 taken8;
} WorkUnit;

#define Vector_Type WorkUnit
//#define Vector_Malloc(bytes) aligned_alloc(alignof(WorkUnit), bytes)
#include "ctl/containers/vector.h"

RenderCtx* Render_New(Scene* scene, ImageRGB* img, Camera* cam)
{
    RenderCtx* ctx = malloc(sizeof(*ctx));

    if (ctx == NULL) {
        return NULL;
    }

    ctx->cam   = cam;
    ctx->scene = scene;
    ctx->img   = img;

    ctx->waiter_args   = NULL;
    ctx->waiter_thread = NULL;

    ctx->finished = true;

    return ctx;
}

// TODO: rethink this part of the API, we should be able to reuse a render context (I think)
void Render_Delete(RenderCtx* ctx)
{
    if (ctx->waiter_thread) {
        Thread_Delete(ctx->waiter_thread);
    }

    free(ctx->waiter_args);
    free(ctx);
}

intern Color RayColor(Scene* scene, Ray* ray, size_t depth)
{
    if (depth == 0) {
        return COLOR_BLACK;
    }

    Object* objHit = NULL;
    HitInfo hit;

    if (Scene_ClosestHit(scene, ray, &objHit, &hit)) {
        Ray   bouncedRay;
        Color surfaceColor;
        Color emittedColor;

        if (Material_Bounce(objHit->material, ray, &hit, &surfaceColor, &emittedColor, &bouncedRay)) {
            return Color_BrightenBy(emittedColor, Color_Tint(surfaceColor, RayColor(scene, &bouncedRay, depth - 1)));
        } else {
            return emittedColor;
        }
    }

    return Scene_Get_SkyColor(scene, ray->dir);
}

typedef struct {
    RenderCtx*        ctx;
    Vector(WorkUnit)* work;

    struct {
        size_t samples_per_pixel;
        size_t max_ray_depth;
    } params;
} RenderThreadArg;

intern RGB RenderPixel(
    Camera* cam,
    Scene*  scene,
    size_t  samples_per_pixel,
    size_t  max_ray_bounces,
    size_t  image_width,
    size_t  image_height,
    size_t  xx,
    size_t  yy)
{
    Color cum_color = {0};

    for (size_t samples = 0; samples < samples_per_pixel; samples++) {
        f32 horizontal_fraction = (xx + Random_Unilateral()) / (f32)(image_width - 1);
        f32 vertical_fraction   = (yy + Random_Unilateral()) / (f32)(image_height - 1);

        Ray   ray       = Camera_GetRay(cam, horizontal_fraction, vertical_fraction);
        Color ray_color = RayColor(scene, &ray, max_ray_bounces);
        cum_color       = vadd(cum_color, ray_color);
    }

    Color avg_color = vdiv(cum_color, samples_per_pixel);
    return RGB_FromColor(avg_color);
}

intern void RenderTile(Camera* cam, Scene* scene, ImageRGB* img, Tile* tile, size_t spp, size_t md)
{
    for (size_t yy = tile->y; yy < tile->y + tile->h; yy++) {
        for (size_t xx = tile->x; xx < tile->x + tile->w; xx++) {
            RGB pixel = RenderPixel(cam, scene, spp, md, img->res.width, img->res.height, xx, yy);
            ImageRGB_SetPixel(img, xx, yy, pixel);
        }
    }
}

intern bool AtomicClaimTile(RenderCtx* ctx, Vector(WorkUnit)* work, size_t x, size_t y)
{
    // convert (x, y) tile coords to the (array, bit) index
    size_t tile_w      = RENDER_TILE_W_PX;
    size_t num_tiles_w = (ctx->img->res.width + tile_w - 1) / tile_w;

    size_t linear_index = y * num_tiles_w + x;
    size_t array_index  = linear_index / 8;
    size_t bit_index    = linear_index % 8;

    u8 claim_mask = 1 << bit_index;
    if (!(claim_mask & __atomic_fetch_or(&work->at[array_index].taken8, claim_mask, __ATOMIC_RELAXED))) {
        return true;
    } else {
        return false;
    }
}

intern void Render_Worker(void* arg)
{
    Random_Seed_HighEntropy();

    RenderThreadArg* args = arg;

    Vector(WorkUnit)* work  = args->work;
    RenderCtx*        ctx   = args->ctx;
    ImageRGB*         img   = ctx->img;
    Camera*           cam   = ctx->cam;
    Scene*            scene = ctx->scene;

    size_t spp = args->params.samples_per_pixel;
    size_t md  = args->params.max_ray_depth;

    // convert (x, y) tile coords to the (array, bit) index
    size_t tile_w      = RENDER_TILE_W_PX;
    size_t tile_h      = RENDER_TILE_H_PX;
    size_t num_tiles_w = (img->res.width + tile_w - 1) / tile_w;
    size_t num_tiles_h = (img->res.height + tile_h - 1) / tile_h;

    // iterate over the tiles, try to claim them, if claimed render the tile
    for (size_t yy = 0; yy < num_tiles_h; yy++) {
        for (size_t xx = 0; xx < num_tiles_w; xx++) {
            if (AtomicClaimTile(ctx, work, xx, yy)) {
                Tile tile = {
                    .x = xx * tile_w,
                    .y = yy * tile_h,
                    .w = MIN(tile_w, img->res.width - xx * tile_w),
                    .h = MIN(tile_h, img->res.height - yy * tile_h),
                };
                RenderTile(cam, scene, img, &tile, spp, md);
            }
        }
    }

    return;
}

typedef struct {
    RenderCtx* ctx;
    size_t     samples_per_pixel;
    size_t     max_ray_depth;
} WaiterThreadArg;

intern void Render_Waiter(void* arg)
{
    // grab args
    WaiterThreadArg* args = arg;

    RenderCtx* ctx               = args->ctx;
    size_t     samples_per_pixel = args->samples_per_pixel;
    size_t     max_ray_depth     = args->max_ray_depth;

    // create queue of work units
    size_t tile_w = RENDER_TILE_W_PX, tile_h = RENDER_TILE_H_PX;
    size_t num_tiles_w = (ctx->img->res.width + tile_w - 1) / tile_w;
    size_t num_tiles_h = (ctx->img->res.height + tile_h - 1) / tile_h;

    Vector(WorkUnit) vect;
    Vector_Init(&vect, num_tiles_w * num_tiles_h);
    Vector_ExtendBy(&vect, num_tiles_w * num_tiles_h);

    // create worker threads
    size_t num_threads    = NUM_HYPERTHREADS;
    size_t min_stack_size = 2 * 1024 * 1024;

    Thread*         threads[NUM_HYPERTHREADS];
    RenderThreadArg thread_args[NUM_HYPERTHREADS];

    for (size_t ii = 0; ii < num_threads; ii++) {
        threads[ii] = Thread_New();
        if (!threads[ii]) {
            ABORT("Failed to create render worker thread");
        }

        Thread_Set_StackSize(threads[ii], min_stack_size);

        thread_args[ii].ctx                      = ctx;
        thread_args[ii].work                     = &vect;
        thread_args[ii].params.max_ray_depth     = max_ray_depth;
        thread_args[ii].params.samples_per_pixel = samples_per_pixel;

        if (!Thread_Spawn(threads[ii], Render_Worker, &thread_args[ii])) {
            ABORT("Failed to start render worker thread");
        }
    }

    // wait for worker threads to finish
    for (size_t ii = 0; ii < num_threads; ii++) {
        Thread_Join(threads[ii]);
        Thread_Delete(threads[ii]);
    }

    Vector_Uninit(&vect);
    ctx->finished = true;
}

void Start_WaiterThread(RenderCtx* ctx, size_t samples_per_pixel, size_t max_ray_depth)
{
    WaiterThreadArg* waiter_args = calloc(1, sizeof(*waiter_args));
    if (waiter_args == NULL) {
        ABORT("Failed to create render waiter thread args");
    }

    waiter_args->ctx               = ctx;
    waiter_args->max_ray_depth     = max_ray_depth;
    waiter_args->samples_per_pixel = samples_per_pixel;

    Thread* waiter_thread = Thread_New();
    if (waiter_thread == NULL) {
        ABORT("Failed to create render waiter thread");
    }

    Thread_Set_StackSize(waiter_thread, 64 * 1024);

    ctx->waiter_thread = waiter_thread;
    ctx->waiter_args   = waiter_args;

    if (!Thread_Spawn(waiter_thread, Render_Waiter, waiter_args)) {
        ABORT("Failed to start render waiter thread");
    }
}

void Render_Start(RenderCtx* ctx, size_t samples_per_pixel, size_t max_ray_depth)
{
    ctx->finished = false;
    Start_WaiterThread(ctx, samples_per_pixel, max_ray_depth);
}

bool Render_Done(RenderCtx* ctx)
{
    return ctx->finished;
}
