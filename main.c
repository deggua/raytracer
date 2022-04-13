#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "common/cext.h"
#include "gfx/camera.h"
#include "gfx/color.h"
#include "gfx/image.h"
#include "gfx/mesh.h"
#include "gfx/primitives.h"
#include "gfx/random.h"
#include "gfx/renderer.h"
#include "gfx/utils.h"
#include "object/object.h"
#include "object/scene.h"

Material diffuse = {
    .type = MATERIAL_DIFFUSE,
    .diffuse = {
        .albedo = COLOR_NAVY,
    },
};

Material metal = {
    .type = MATERIAL_METAL,
    .metal = {
        .albedo = (Color){0.7f, 0.6f, 0.5f},
        .fuzz = 0.0f,
    },
};

Material glass = {
    .type = MATERIAL_DIELECTRIC,
    .dielectric = {
        .albedo = COLOR_WHITE,
        .refactiveIndex = 1.5f,
    },
};

Material ground = {
    .type = MATERIAL_DIFFUSE,
    .diffuse = {
        .albedo = COLOR_GREY,
    },
};

static void ExportImage(Image* img, const char* filename)
{
    FILE* fd = fopen(filename, "w+");
    Image_ExportBMP(img, fd);
    printf("Output image written to %s\n", filename);
}

Image*      g_img;
static void InterruptHandler(int sig)
{
    (void)sig;
    printf("\nExporting partial image to disk\n");
    ExportImage(g_img, "partial.bmp");
    exit(EXIT_SUCCESS);
}

static void FillScene(Scene* scene)
{
    // mesh
#if 0
    FILE* fd   = fopen("assets/teapot.obj", "r");
    Mesh* mesh = Mesh_New((Point3){0, 1, 0}, 2.5f, &diffuse);
#endif

#if 1
    FILE* fd   = fopen("assets/dragon.obj", "r");
    Mesh* mesh = Mesh_New((Point3){0, 7, 0}, 1.0f / 10.0f, &glass);
#endif

    Mesh_LoadOBJ(mesh, fd);
    Mesh_AddToScene(mesh, scene);
    Mesh_Delete(mesh);

    // ground
    Object obj;
    obj.material       = &ground;
    obj.surface.type   = SURFACE_SPHERE;
    obj.surface.sphere = Sphere_Make((Point3){0, -1000, 0}, 1000.0f);
    Scene_Add_Object(scene, &obj);
}

int main(void)
{
    const Point3 lookFrom    = (Point3){20, 12, 20};
    const Point3 lookAt      = (Point3){0, 4, 0};
    const Vec3   vup         = (Vec3){0, 1, 0};
    const float  focusDist   = 10.0f;
    const float  aperature   = 0.0f;
    const float  aspectRatio = 16.0f / 9.0f;
    const float  vFov        = 40.0f;
    const float  timeStart   = 0.0f;
    const float  timeEnd     = 1.0f;

    const size_t imageHeight = 720;
    const size_t imageWidth  = imageHeight * aspectRatio;

    const size_t numThreads = 4;

    struct timespec specStart;
    struct timespec specEnd;

    signal(SIGINT, InterruptHandler);
    Random_Seed(__builtin_readcyclecounter());

    Image* img = Image_New(imageWidth, imageHeight);
    g_img      = img;

    Camera* cam   = Camera_New(lookFrom, lookAt, vup, aspectRatio, vFov, aperature, focusDist, timeStart, timeEnd);
    Scene*  scene = Scene_New(COLOR_WHITE);

    printf("Filling scene with objects\n");
    FillScene(scene);

    printf("Preparing scene\n");
    Scene_Prepare(scene);

    RenderCtx* ctx = RenderCtx_New(scene, img, cam);

    // Time the render
    printf("Rendering scene\n");
    {
        clock_gettime(CLOCK_MONOTONIC, &specStart);
        // TODO: does it make sense for Render to create the image?
        // TODO: pass in worker thread stack size? or could we compute the required stack size from the ray depth?
        Render(ctx, 64, 32, numThreads);

        clock_gettime(CLOCK_MONOTONIC, &specEnd);
        float timeDeltaSec  = (float)(specEnd.tv_sec - specStart.tv_sec);
        float timeDeltaFrac = (float)(specEnd.tv_nsec - specStart.tv_nsec) * 1e-9;
        float timeDelta     = timeDeltaSec + timeDeltaFrac;
        printf("\n%.2f seconds spent rendering\n", timeDelta);
    }

    ExportImage(img, "output.bmp");

    Image_Delete(img);
    Scene_Delete(scene);
    RenderCtx_Delete(ctx);

    return 0;
}
