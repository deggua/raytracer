#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#include "common/common.h"
#include "common/math.h"
#include "common/profiling.h"
#include "common/random.h"
#include "common/vec.h"
#include "gfx/color.h"
#include "gfx/image.h"
#include "gfx/mesh.h"
#include "rt/renderer.h"
#include "world/camera.h"
#include "world/object.h"
#include "world/scene.h"

Material diffuse = {
    .type    = MATERIAL_DIFFUSE,
    .diffuse = {.albedo = COLOR_GREEN},
};

Material metal = {
    .type  = MATERIAL_METAL,
    .metal = {
        .albedo = COLOR_RED,
        .fuzz = 0.4f
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
    Image_Export_BMP(img, fd);
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
#    if 0
    FILE* fd   = fopen("assets/teapot.obj", "r");
    Mesh* mesh = Mesh_New((point3) {0, 1, 0}, 2.5f, &diffuse);
#    endif

#    if 1
    FILE* fd   = fopen("assets/dragon.obj", "r");
    Mesh* mesh = Mesh_New();
    Mesh_Import_OBJ(mesh, fd);

    Mesh_Set_Origin(mesh, (point3) {{0, 7, 10}});
    Mesh_Set_Scale(mesh, 1.0f / 10.0f);
    Mesh_Set_Material(mesh, &diffuse);
    Mesh_AddToScene(mesh, scene);

    Mesh_Set_Origin(mesh, (point3) {{0, 7, 0}});
    Mesh_Set_Material(mesh, &glass);
    Mesh_AddToScene(mesh, scene);

    Mesh_Set_Origin(mesh, (point3) {.v = {0, 7, -10}});
    Mesh_Set_Material(mesh, &metal);
    Mesh_AddToScene(mesh, scene);

    Mesh_Delete(mesh);
#    endif

    // ground
    Object obj;
    obj.material       = &ground;
    obj.surface.type   = SURFACE_SPHERE;
    obj.surface.sphere = Sphere_Make((point3) {0, -1000, 0}, 1000.0f);
    Scene_Add_Object(scene, &obj);
}

int main(void)
{
    const point3 lookFrom    = (point3) {20, 14, 20};
    const point3 lookAt      = (point3) {0, 6, 0};
    const vec3   vup         = (vec3) {0, 1, 0};
    const f32    focusDist   = 10.0f;
    const f32    aperature   = 0.0f;
    const f32    aspectRatio = 16.0f / 9.0f;
    const f32    vFov        = 40.0f;
    const f32    timeStart   = 0.0f;
    const f32    timeEnd     = 1.0f;

    const size_t imageHeight = 720;
    const size_t imageWidth  = imageHeight * aspectRatio;

    const size_t numThreads = 4;

    signal(SIGINT, InterruptHandler);
    RNG_Seed(__builtin_readcyclecounter());

    Image* img = Image_New(imageWidth, imageHeight);
    g_img      = img;

    Camera* cam   = Camera_New(lookFrom, lookAt, vup, aspectRatio, vFov, aperature, focusDist, timeStart, timeEnd);
    Scene*  scene = Scene_New(COLOR_WHITE);

    TIMEIT("Scene load", FillScene(scene));
    TIMEIT("Scene prepare", Scene_Prepare(scene));

    RenderCtx* ctx = Render_New(scene, img, cam);

    TIMEIT("Scene render", Render_Do(ctx, 64, 32, numThreads));

    ExportImage(img, "output.bmp");

    Image_Delete(img);
    Scene_Delete(scene);
    Render_Delete(ctx);

    return 0;
}
