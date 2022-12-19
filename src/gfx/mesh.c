#include <stdio.h>
#include <string.h>

#include "math/vec.h"
#include "rt/materials.h"
#include "rt/surfaces.h"
#include "world/scene.h"

#define Vector_Type Triangle
#include "ctl/containers/vector.h"

#define Vector_Type point3
#include "ctl/containers/vector.h"

#define Vector_Type point2
#include "ctl/containers/vector.h"

typedef struct Mesh {
    point3            origin;
    f32               scale;
    Material*         material;
    Vector(Triangle)* polys;
} Mesh;

Mesh* Mesh_New(void)
{
    Mesh* mesh = (Mesh*)calloc(1, sizeof(*mesh));

    if (mesh == NULL) {
        goto error_Mesh;
    }

    mesh->polys = Vector_New(Triangle)(16);
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
    Vector_Delete(mesh->polys);
    free(mesh);
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

void Mesh_AddToScene(Mesh* mesh, Scene* scene)
{
    Object obj;

    for (size_t ii = 0; ii < mesh->polys->length; ii++) {
        Triangle triWorldSpace = mesh->polys->at[ii];

        for (size_t jj = 0; jj < 3; jj++) {
            triWorldSpace.vtx[jj].pos = vadd(vmul(triWorldSpace.vtx[jj].pos, mesh->scale), mesh->origin);
        }

        obj.material         = mesh->material;
        obj.surface.type     = SURFACE_TRIANGLE;
        obj.surface.triangle = triWorldSpace;

        Scene_Add_Object(scene, &obj);
    }
}

// TODO: error handling
bool Mesh_Import_OBJ(Mesh* mesh, FILE* fd)
{
    char lineBuffer[512];

    Vector(point3) vertices;
    if (!Vector_Init(&vertices, 16)) {
        goto error_VectorVertices;
    }

    Vector(point2) texCoords;
    if (!Vector_Init(&texCoords, 16)) {
        goto error_VectorTexCoords;
    }

    Vector(point3) vertexNormals;
    if (!Vector_Init(&vertexNormals, 16)) {
        goto error_VectorVertexNormals;
    }

    // first pass gets all the vertices/texture coords/normals
    while (!feof(fd)) {
        point3 vertexNormal;
        point3 vertex;
        point2 texCoord;

        fgets(lineBuffer, sizeof(lineBuffer), fd);

        if (sscanf(lineBuffer, "v %f %f %f\n", &vertex.x, &vertex.y, &vertex.z) == 3) {
            Vector_Push(&vertices, &vertex);
        } else if (sscanf(lineBuffer, "vt %f %f\n", &texCoord.x, &texCoord.y) == 2) {
            Vector_Push(&texCoords, &texCoord);
        } else if (sscanf(lineBuffer, "vn %f %f %f\n", &vertexNormal.x, &vertexNormal.y, &vertexNormal.z) == 3) {
            Vector_Push(&vertexNormals, &vertexNormal);
        }
    }

    printf("Found %zu vertices\n", vertices.length);
    printf("Found %zu texture coordinates\n", texCoords.length);
    printf("Found %zu vertex normals\n", vertexNormals.length);

    // second pass constructs the triangles
    fseek(fd, 0, SEEK_SET);

    // TODO: refactor this into something sane
    while (!feof(fd)) {
        fgets(lineBuffer, sizeof(lineBuffer), fd);

        if (strncmp(lineBuffer, "f ", 2) == 0) {
            // extract values
            ssize_t vi[4] = {[0 ... 3] = -1};
            ssize_t vn[4] = {[0 ... 3] = -1};
            ssize_t vt[4] = {[0 ... 3] = -1};
            size_t  tc;

            // remove the 'f' token
            char* token = strtok(lineBuffer, " ");
            token       = strtok(NULL, " ");

            for (tc = 0; token != NULL; tc++) {
                if (sscanf(token, "%zu/%zu/%zu", &vi[tc], &vt[tc], &vn[tc]) == 3) {
                    goto next_token;
                } else if (sscanf(token, "%zu//%zu", &vi[tc], &vn[tc]) == 2) {
                    goto next_token;
                } else if (sscanf(token, "%zu/%zu", &vi[tc], &vt[tc]) == 2) {
                    goto next_token;
                } else if (sscanf(token, "%zu", &vi[tc]) == 1) {
                    goto next_token;
                } else {
                    ABORT("No match for token \"%s\"", token);
                }

next_token:
                token = strtok(NULL, " ");
            }

            if (tc == 4) {
                // need to split into two triangles
                Triangle tri[2];
                size_t   indices[2][3] = {
                    {0, 1, 2},
                    {0, 2, 3}
                };

                for (size_t ii = 0; ii < 2; ii++) {
                    // first pass is just the vertex positions since they must be valid
                    for (size_t jj = 0; jj < 3; jj++) {
                        ssize_t index       = indices[ii][jj];
                        tri[ii].vtx[jj].pos = vertices.at[vi[index] - 1];
                    }

                    // since a file can omit normals we need to calculate a default normal based on the edges if
                    // it isn't provided
                    vec3 edge1         = vsub(tri[ii].vtx[1].pos, tri[ii].vtx[0].pos);
                    vec3 edge2         = vsub(tri[ii].vtx[2].pos, tri[ii].vtx[0].pos);
                    vec3 defaultNormal = vcross(edge1, edge2);

                    for (size_t jj = 0; jj < 3; jj++) {
                        ssize_t index = indices[ii][jj];

                        if (vn[index] >= 0) {
                            tri[ii].vtx[jj].norm = vertexNormals.at[vn[index] - 1];
                        } else {
                            tri[ii].vtx[jj].norm = defaultNormal;
                        }

                        if (vt[index] >= 0) {
                            tri[ii].vtx[jj].tex = texCoords.at[vt[index] - 1];
                        } else {
                            tri[ii].vtx[jj].tex = (point2){0.0f, 0.0f};
                        }
                    }
                }

                Vector_Push(mesh->polys, &tri[0]);
                Vector_Push(mesh->polys, &tri[1]);
            } else if (tc == 3) {
                // it's a single triangle
                Triangle tri;

                for (size_t ii = 0; ii < 3; ii++) {
                    tri.vtx[ii].pos = vertices.at[vi[ii] - 1];
                }

                // since a file can omit normals we need to calculate a default normal based on the edges if
                // it isn't provided
                vec3 edge1         = vsub(tri.vtx[1].pos, tri.vtx[0].pos);
                vec3 edge2         = vsub(tri.vtx[2].pos, tri.vtx[0].pos);
                vec3 defaultNormal = vnorm(vcross(edge1, edge2));

                for (size_t ii = 0; ii < 3; ii++) {
                    if (vn[ii] >= 0) {
                        tri.vtx[ii].norm = vertexNormals.at[vn[ii] - 1];
                    } else {
                        tri.vtx[ii].norm = defaultNormal;
                    }

                    if (vt[ii] >= 0) {
                        tri.vtx[ii].tex = texCoords.at[vt[ii] - 1];
                    } else {
                        tri.vtx[ii].tex = (point2){0.0f, 0.0f};
                    }
                }

                Vector_Push(mesh->polys, &tri);
            }
        }
    }

    printf("Found %zu tris\n", mesh->polys->length);

    Vector_Uninit(&vertexNormals);
error_VectorVertexNormals:
    Vector_Uninit(&texCoords);
error_VectorTexCoords:
    Vector_Uninit(&vertices);
error_VectorVertices:
    return true;
}
