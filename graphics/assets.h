#if !defined(ASSETS_H)
#include "base_def.h"

#undef global
#undef local_global

#include "graphics_math.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#define global static
#define local_global static

struct vertex
{
    v3 Pos;
    v2 Uv;
};

struct texture
{
    i32 Width;
    i32 Height;
    u32* Texels;
};

struct mesh
{
    u32 IndexOffset;
    u32 IndexCount;

    u32 VertexOffset;
    u32 VertexCount;

    u32 TextureId;
};

struct model
{
    u32 NumMeshes;
    mesh* MeshArray;

    u32 VertexCount;
    vertex* VertexArray;

    u32 IndexCount;
    u32* IndexArray;

    u32 NumTextures;
    texture* TextureArray;
};

model AssetLoadModel(char* FolderPath, char* Name);

#define ASSETS_H
#endif // !defined(ASSETS_H)
