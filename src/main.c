#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/math.h"
#include "common/random.h"
#include "common/vec.h"
#include "gfx/color.h"
#include "gfx/image.h"
#include "gfx/mesh.h"
#include "platform/profiling.h"
#include "rt/renderer.h"
#include "world/camera.h"
#include "world/object.h"
#include "world/scene.h"

static void ExportImage(Image* img, const char* filename)
{
    FILE* fd = fopen(filename, "wb+");
    Image_Export_BMP(img, fd);
    // Image_Export_PPM(img, fd);
    printf("Output image written to %s\n", filename);
}

Image* g_img;

static void InterruptHandler(int sig)
{
    (void)sig;
    printf("\nExporting partial image to disk\n");
    ExportImage(g_img, "partial.bmp");
    exit(EXIT_SUCCESS);
}

Material g_matMesh;
Material g_matMesh2;
Material g_matGround;
Material g_matLight;

static void FillScene(Scene* scene)
{
    /* Pear Mesh */
#if 0
    FILE* obj = fopen("assets/pear/obj/pear_export.obj", "r");
    FILE* tex = fopen("assets/pear/tex/pear_diffuse.bmp", "rb");

    Mesh* mesh = Mesh_New();
    Mesh_Import_OBJ(mesh, obj);
    fclose(obj);

    Texture* texMesh = Texture_New();
    Texture_Import_BMP(texMesh, tex);
    fclose(tex);

    g_matMesh.type = MATERIAL_DIFFUSE;
    g_matMesh.diffuse.albedo = texMesh;

    Mesh_Set_Material(mesh, &g_matMesh);
    Mesh_Set_Origin(mesh, (point3) {0, 6, -4});
    Mesh_Set_Scale(mesh, 2.0f);
    Mesh_AddToScene(mesh, scene);
#else

    /* Little Dragon Mesh */
    FILE* littleDragon = fopen("assets/little_dragon.obj", "r");
    if (littleDragon == NULL) {
        printf("Couldn't find assets/little_dragon.obj\n");
        exit(EXIT_FAILURE);
    }

    Mesh* mesh = Mesh_New();
    Mesh_Import_OBJ(mesh, littleDragon);
    fclose(littleDragon);

    Texture* texMesh = Texture_New();
    Texture_Import_Color(texMesh, COLOR_NAVY);

    g_matMesh.type  = MATERIAL_METAL;
    g_matMesh.metal = Metal_Make(texMesh, 0.0f);

    Mesh_Set_Material(mesh, &g_matMesh);
    Mesh_Set_Origin(mesh, (point3){0, 1, 0});
    Mesh_Set_Scale(mesh, 1 / 10.0f);
    Mesh_AddToScene(mesh, scene);

    Mesh_Delete(mesh);
#endif

    /* Sphere Light */
    Texture* texLight = Texture_New();
    Texture_Import_Color(texLight, COLOR_WHITE);
    g_matLight.type                    = MATERIAL_DIFFUSE_LIGHT;
    g_matLight.diffuseLight.albedo     = texLight;
    g_matLight.diffuseLight.brightness = 5.0f;

    Object lightObj;
    lightObj.material         = &g_matLight;
    lightObj.surface.type     = SURFACE_SPHERE;
    lightObj.surface.sphere.c = (point3){4, 24, 4};
    lightObj.surface.sphere.r = 6.0f;
    Scene_Add_Object(scene, &lightObj);

    /* Ground */
    Texture* texGround = Texture_New();
    Texture_Import_Color(texGround, COLOR_GREY);
    g_matGround.type           = MATERIAL_DIFFUSE;
    g_matGround.diffuse.albedo = texGround;

    Object worldObj;
    worldObj.material       = &g_matGround;
    worldObj.surface.type   = SURFACE_SPHERE;
    worldObj.surface.sphere = Sphere_Make((point3){0, -1000, 0}, 1000.0f);
    Scene_Add_Object(scene, &worldObj);
}

int main(int argc, char** argv)
{
    const point3 lookFrom    = (point3){20, 15, 20};
    const point3 lookAt      = (point3){0, 6, 0};
    const vec3   vup         = (vec3){0, 1, 0};
    const f32    focusDist   = 10.0f;
    const f32    aperature   = 0.0f;
    const f32    aspectRatio = 16.0f / 9.0f;
    const f32    vFov        = 40.0f;
    const f32    timeStart   = 0.0f;
    const f32    timeEnd     = 1.0f;

    const size_t imageHeight = 720;
    const size_t imageWidth  = imageHeight * aspectRatio;

    size_t numThreads      = 16;
    size_t samplesPerPixel = 32;
    size_t maxBounces      = 12;

    if (argc > 1)
        numThreads = atol(argv[1]);
    if (argc > 2)
        samplesPerPixel = atol(argv[2]);
    if (argc > 3)
        maxBounces = atol(argv[3]);

    signal(SIGINT, InterruptHandler);
    Random_Seed(__builtin_readcyclecounter());

    Image* img = Image_New(imageWidth, imageHeight);
    g_img      = img;

    Camera* cam   = Camera_New(lookFrom, lookAt, vup, aspectRatio, vFov, aperature, focusDist, timeStart, timeEnd);
    Scene*  scene = Scene_New();
    Scene_Set_SkyColor(scene, (Color){0.00f, 0.00f, 0.00f});

    TIMEIT("Scene load", FillScene(scene));
    TIMEIT("Scene prepare", Scene_Prepare(scene));

    RenderCtx* ctx = Render_New(scene, img, cam);

    TIMEIT("Scene render", Render_Do(ctx, samplesPerPixel, maxBounces, numThreads));

    ExportImage(img, "output.bmp");

    Image_Delete(img);
    Scene_Delete(scene);
    Render_Delete(ctx);

    return 0;
}
