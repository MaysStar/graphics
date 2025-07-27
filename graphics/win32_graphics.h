#if !defined(WIN32_GRAPHICS_H)

#include "base_def.h"
#include "clipper.h"

struct texture
{
    u32 Width;
    u32 Height;
    u32* Texels;
};

enum sampler_type
{
    SamplerType_None,
    SamplerType_Nearest,
    SamplerType_Bilinear,
};

struct sampler
{
    sampler_type Type;
    u32 BorderColor;
};

struct camera
{
    v2 PrevMousePos;
    v3 Pos;
    b32 PrevMouseDown;
    f32 Yaw;
    f32 Pitch;
};

struct global_state
{
    b32 IsRunning;
    HWND WindowHandle;
    HDC DeviceContext;
    u32 FrameBufferStride;
    u32 FrameBufferWidth;
    u32 FrameBufferHeight;
    u32* FrameBufferPixels;

    f32 CursorOffset;
    f32* DepthBuffer;

    f32 CurrTime;

    camera Camera;

    b32 WDown;
    b32 ADown;
    b32 SDown;
    b32 DDown;
};

#define WIN32_GRAPHICS_H
#endif