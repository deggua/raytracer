#include "renderer.h"

#include <math.h>
#include <stdlib.h>

#include "math/math.h"
#include "math/random.h"
#include "platform/threads.h"

RenderCtx* Render_New(in Scene* scene, in ImageRGB* img, in Camera* cam)
{
    RenderCtx* ctx = malloc(sizeof(*ctx));

    if (ctx == NULL) {
        return NULL;
    }

    ctx->cam   = cam;
    ctx->scene = scene;
    ctx->img   = img;

    return ctx;
}

void Render_Delete(out RenderCtx* ctx)
{
    free(ctx);
}

intern Color RayColor(in Scene* scene, in Ray* ray, size_t depth)
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
    RenderCtx* ctx;

    struct {
        size_t samples_per_pixel;
        size_t max_ray_depth;
        size_t line_offset;
        size_t line_increment;
    } params;
} RenderThreadArg;

intern void Render_Worker(void* arg)
{
    RenderThreadArg* args = arg;

    Random_Seed_HighEntropy();

    ImageRGB* img   = args->ctx->img;
    Camera*   cam   = args->ctx->cam;
    Scene*    scene = args->ctx->scene;

    size_t line_increment = args->params.line_increment;
    size_t line_offset    = args->params.line_offset;

    size_t samples_per_pixel = args->params.samples_per_pixel;
    size_t max_ray_depth     = args->params.max_ray_depth;

    size_t image_height = args->ctx->img->res.height;
    size_t image_width  = args->ctx->img->res.width;

    // compute the image
    for (i64 yy = image_height - 1 - line_offset; yy >= 0; yy -= line_increment) {
        for (size_t xx = 0; xx < image_width; xx++) {
            Color cum_color = {0};

            for (size_t samples = 0; samples < samples_per_pixel; samples++) {
                f32 horizontal_fraction = (xx + Random_Unilateral()) / (f32)(image_width - 1);
                f32 vertical_fraction   = (yy + Random_Unilateral()) / (f32)(image_height - 1);

                Ray   ray       = Camera_GetRay(cam, horizontal_fraction, vertical_fraction);
                Color ray_color = RayColor(scene, &ray, max_ray_depth);
                cum_color       = vadd(cum_color, ray_color);
            }

            Color avg_color = vdiv(cum_color, samples_per_pixel);
            RGB   avg_sRGB  = RGB_FromColor(avg_color);

            ImageRGB_SetPixel(img, xx, image_height - 1 - yy, avg_sRGB);
        }

        // TODO: figure out a better way to print progress
        printf("\rFinished line %03zd of %zu", yy, image_height);
        fflush(stdout);
    }

    return;
}

// TODO: error checking on allocations
ImageRGB* Render_Do(in RenderCtx* ctx, size_t samples_per_pixel, size_t max_ray_depth, size_t num_threads)
{
    size_t min_stack_size = 16 * 1024 * 1024;

    Thread**         threads     = calloc(num_threads, sizeof(Thread*));
    RenderThreadArg* thread_args = calloc(num_threads, sizeof(*thread_args));

    for (size_t ii = 0; ii < num_threads; ii++) {
        threads[ii] = Thread_New();
        Thread_Set_StackSize(threads[ii], min_stack_size);

        thread_args[ii].ctx                      = ctx;
        thread_args[ii].params.max_ray_depth     = max_ray_depth;
        thread_args[ii].params.samples_per_pixel = samples_per_pixel;
        thread_args[ii].params.line_increment    = num_threads;
        thread_args[ii].params.line_offset       = ii;

        Thread_Spawn(threads[ii], Render_Worker, &thread_args[ii]);
    }

    for (size_t ii = 0; ii < num_threads; ii++) {
        Thread_Join(threads[ii]);
    }

    printf("\nFinished rendering\n");

    for (size_t ii = 0; ii < num_threads; ii++) {
        Thread_Delete(threads[ii]);
    }

    free(threads);
    free(thread_args);

    return ctx->img;
}
