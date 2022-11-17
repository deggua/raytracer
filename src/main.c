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

static bool ExportImage(ImageRGB* img, const char* filename)
{
    FILE* fd = fopen(filename, "wb+");
    if (fd == NULL) {
        printf("Failed to export render: Cannot open %s\n", filename);
        return false;
    }

    if (!ImageRGB_Save_BMP(img, fd)) {
        fclose(fd);
        printf("Failed to export render: Cannot write to %s\n", filename);
        return false;
    }

    fclose(fd);
    printf("Exported render to %s\n", filename);

    return true;
}

ImageRGB* g_img;

static void InterruptHandler(int sig)
{
    (void)sig;

    printf("\nExporting partial image to disk\n");

    if (ExportImage(g_img, "partial.bmp")) {
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}

Material g_matMesh;
Material g_matMesh2;
Material g_matGround;
Material g_matLight;

static void FillScene(Scene* scene, Skybox* skybox)
{
#if 0
    /* Pear Mesh */
    FILE* obj = fopen("assets/pear/obj/pear_export.obj", "r");
    FILE* tex = fopen("assets/pear/tex/pear_diffuse.bmp", "rb");

    Mesh* mesh = Mesh_New();
    Mesh_Import_OBJ(mesh, obj);
    fclose(obj);

    Texture* texMesh = Texture_New();
    Texture_Import_BMP(texMesh, tex);
    fclose(tex);

    g_matMesh.type           = MATERIAL_DIFFUSE;
    g_matMesh.diffuse.albedo = texMesh;

    Mesh_Set_Material(mesh, &g_matMesh);
    Mesh_Set_Origin(mesh, (point3){0, 6, -4});
    Mesh_Set_Scale(mesh, 2.0f);
    Mesh_AddToScene(mesh, scene);

#elif 1

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
    Texture_Import_Color(texMesh, COLOR_GREY);

    g_matMesh = Material_Test_Make(texMesh);
    // g_matMesh = Material_Diffuse_Make(texMesh);

    Mesh_Set_Material(mesh, &g_matMesh);
    Mesh_Set_Origin(mesh, (point3){0, 1, 0});
    Mesh_Set_Scale(mesh, 1 / 10.0f);
    Mesh_AddToScene(mesh, scene);

    // Mesh_Set_Origin(mesh, (point3){-15, 1, 0});
    // Mesh_AddToScene(mesh, scene);

    Mesh_Delete(mesh);

#else
    /* Shiny Sphere */
    Texture* tex = Texture_New();
    Texture_Import_Color(tex, COLOR_GREY);
    g_matMesh = Material_Metal_Make(tex, 0.0f);

    Object sphere = {
        .material = &g_matMesh,
        .surface = {
            .type = SURFACE_SPHERE,
            .sphere = {
                .r = 6.0f,
                .c = (point3){0, 6, 0},
            },
        },
    };
    Scene_Add_Object(scene, &sphere);
#endif

#if 1
    /* Sphere Light */
    Texture* texLight = Texture_New();
    // c3d7f0
    Texture_Import_Color(texLight, Color_FromRGB((RGB){0xc3, 0xd7, 0xf0}));
    g_matLight = Material_DiffuseLight_Make(texLight, 5.0f);

    Object lightObj;
    lightObj.material         = &g_matLight;
    lightObj.surface.type     = SURFACE_SPHERE;
    lightObj.surface.sphere.c = (point3){-8, 22, -8};
    lightObj.surface.sphere.r = 4.0f;
    Scene_Add_Object(scene, &lightObj);
#endif

#if 1
    /* Ground */
    // Texture* texGround = Texture_New();
    // Texture_Import_Color(texGround, COLOR_GREY);
    // g_matGround = Material_Diffuse_Make(texGround);
    g_matGround = Material_Skybox_Make(skybox);

    point3 p1 = {-10000, 0, -10000};
    point3 p2 = {-10000, 0, 10000};
    point3 p3 = {10000, 0, -10000};
    point3 p4 = {10000, 0, 10000};

    Object worldObj1 = {
        .material = &g_matGround,
        .surface = {
            .type = SURFACE_TRIANGLE,
            .triangle = Triangle_MakeSimple(p1, p2, p3),
        },
    };

    Object worldObj2 = {
        .material = &g_matGround,
        .surface = {
            .type = SURFACE_TRIANGLE,
            .triangle = Triangle_MakeSimple(p4, p2, p3),
        },
    };

    Scene_Add_Object(scene, &worldObj1);
    Scene_Add_Object(scene, &worldObj2);
#endif
}

int main(int argc, char** argv)
{
    const point3 lookFrom    = (point3){20, 15, 20};
    const point3 lookAt      = (point3){0, 6, 0};
    const vec3   vup         = (vec3){0, 1, 0};
    f32          focusDist   = vmag(vsub(lookFrom, lookAt));
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

    ImageRGB* img = calloc(1, sizeof(ImageRGB));
    if (img == NULL) {
        printf("Failed to create image container\n");
        exit(EXIT_FAILURE);
    }

    if (!ImageRGB_Load_Empty(img, imageWidth, imageHeight)) {
        printf("Failed to create image buffer\n");
    }

    g_img = img;

    Camera* cam = Camera_New(lookFrom, lookAt, vup, aspectRatio, vFov, aperature, focusDist, timeStart, timeEnd);
    if (cam == NULL) {
        printf("Failed to create camera\n");
        exit(EXIT_FAILURE);
    }

    Skybox* skybox = Skybox_Import_BMP("assets/skybox2");
    if (skybox == NULL) {
        printf("Failed to load skybox\n");
        exit(EXIT_FAILURE);
    }

    Scene* scene = Scene_New(skybox);
    if (scene == NULL) {
        printf("Failed to create scene\n");
        exit(EXIT_FAILURE);
    }

    TIMEIT("Scene load", FillScene(scene, skybox));
    TIMEIT("Scene prepare", Scene_Prepare(scene));

    RenderCtx* ctx = Render_New(scene, img, cam);

    TIMEIT("Scene render", Render_Do(ctx, samplesPerPixel, maxBounces, numThreads));

    // TODO: cleanup on exit

    if (ExportImage(img, "output.bmp")) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
