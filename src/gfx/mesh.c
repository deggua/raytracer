#include <stdio.h>

#include "common/common.h"
#include "common/vec.h"
#include "rt/materials.h"
#include "rt/surfaces.h"
#include "world/scene.h"

#define TEMPLATE_TYPE Triangle
#include "templates/vector.h"

#define TEMPLATE_TYPE point3
#include "templates/vector.h"

typedef struct Mesh {
    point3    origin;
    f32       scale;
    Material* material;
    Vector(Triangle) * polys;
} Mesh;

Mesh* Mesh_New(void)
{
    Mesh* mesh = calloc(1, sizeof(*mesh));
    if (mesh == NULL) {
        goto error_Mesh;
    }

    mesh->polys = Vector_New(Triangle)();
    if (mesh->polys == NULL) {
        goto error_Polys;
    }

    return mesh;

error_Polys:
    free(mesh);
error_Mesh:
    return NULL;
}

void Mesh_Delete(Mesh* mesh)
{
    Vector_Delete(Triangle)(mesh->polys);
    free(mesh);
}

void Mesh_AddToScene(Mesh* mesh, Scene* scene)
{
    Object obj;
    for (size_t ii = 0; ii < mesh->polys->length; ii++) {
        Triangle triWorldSpace = mesh->polys->at[ii];
        for (size_t jj = 0; jj < 3; jj++) {
            triWorldSpace.v[jj] = vadd(vmul(triWorldSpace.v[jj], mesh->scale), mesh->origin);
        }

        obj.material         = mesh->material;
        obj.surface.type     = SURFACE_TRIANGLE;
        obj.surface.triangle = triWorldSpace;

        Scene_Add_Object(scene, &obj);
    }
}

bool Mesh_Import_OBJ(Mesh* mesh, FILE* fd)
{
    Vector(point3)* vertices = Vector_New(point3)();
    fseek(fd, 0, SEEK_SET);
    while (!feof(fd)) {
        point3 tmp;
        if (fscanf(fd, "v %f %f %f\n", &tmp.x, &tmp.y, &tmp.z) == 3) {
            Vector_Push(point3)(vertices, &tmp);
        } else {
            fscanf(fd, "%*s\n");
        }
    }
    printf("Found %zu vertices\n", vertices->length);

    Triangle tri;
    fseek(fd, 0, SEEK_SET);
    while (!feof(fd)) {
        size_t vi1, vi2, vi3;
        if (fscanf(fd, "f %zu %zu %zu\n", &vi1, &vi2, &vi3) == 3) {
            tri.v[0] = vertices->at[vi1 - 1];
            tri.v[1] = vertices->at[vi2 - 1];
            tri.v[2] = vertices->at[vi3 - 1];
            Vector_Push(Triangle)(mesh->polys, &tri);
        } else {
            fscanf(fd, "%*s\n");
        }
    }
    printf("Found %zu tris\n", mesh->polys->length);

    Vector_Delete(point3)(vertices);

    return true;
}

void Mesh_Set_Material(Mesh* mesh, Material* material)
{
    mesh->material = material;
}

void Mesh_Set_Origin(Mesh* mesh, point3 origin)
{
    mesh->origin = origin;
}

void Mesh_Set_Scale(Mesh* mesh, f32 scale)
{
    mesh->scale = scale;
}
