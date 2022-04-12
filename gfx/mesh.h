#include <stdbool.h>
#include <stdio.h>

typedef struct {
} Mesh;

Mesh* Mesh_New(void);
void  Mesh_Delete(Mesh*);
bool  Mesh_LoadOBJ(Mesh* mesh, FILE*);
