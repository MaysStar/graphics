#if !defined(CLIPPER_H)

#define W_CLIPPING_PLANE 0.0001f
#define CLIP_MAX_NUM_VERTICES 384
#include "graphics_math.h"

enum clip_axis
{
    ClipAxis_None,

    ClipAxis_Left,
    ClipAxis_Right,
    ClipAxis_Top,
    ClipAxis_Bottom,
    ClipAxis_Near,
    ClipAxis_Far,
    ClipAxis_W,
};

struct clip_vertex
{
    v4 Pos;
    v2 Uv;
};

struct clip_result
{
    u32 NumTriangles;
    clip_vertex Vertices[CLIP_MAX_NUM_VERTICES];
};

b32 IsBehindPlane(clip_vertex Vertex, clip_axis Axis);
clip_vertex CalculateIntersection(clip_vertex Start, clip_vertex End, clip_axis Axis);
void ClipPolygonToAxis(clip_result* Input, clip_result* Output, clip_axis Axis);

#define CLIPPER_H
#endif