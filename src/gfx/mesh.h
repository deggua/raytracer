#include <stdbool.h>
#include <stdio.h>

#include "common/common.h"
#include "rt/materials.h"
#include "world/scene.h"

typedef struct Mesh Mesh;

Mesh* Mesh_New(void);
void  Mesh_Delete(Mesh* mesh);

bool Mesh_Import_OBJ(Mesh* mesh, FILE*);
void Mesh_AddToScene(Mesh* mesh, Scene* scene);

void Mesh_Set_Material(Mesh* mesh, Material* material);
void Mesh_Set_Origin(Mesh* mesh, point3 origin);
void Mesh_Set_Scale(Mesh* mesh, f32 scale);

// TODO: Mesh_Set_Rotation
