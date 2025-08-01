#include "win32_graphics.h"

global global_state GlobalState;

v2 NdcToPixels(v2 NdcPos)
{
	v2 Result = {};
	Result = 0.5f * (NdcPos + V2(1.0f, 1.0f));
	Result = V2(GlobalState.FrameBufferWidth, GlobalState.FrameBufferHeight) * Result;
	return Result;
}

i64 CrossProduct2d(v2i A, v2i B)
{
    i64 Result = i64(A.x) * i64(B.y) - i64(A.y) * i64(B.x);
    return Result;
}

v3_x4 ColorI32ToRgb(i32_x4 Color)
{
	v3_x4 Result = {};
	Result.r = F32X4((Color >> 16) & 0xFF);
	Result.g = F32X4((Color >> 8) & 0xFF);
	Result.b = F32X4((Color >> 0) & 0xFF);
	Result = Result / 255.0f;
	return Result;
}

i32_x4 ColorRgbToI32(v3_x4 Color)
{
	Color = Color * 255.0f;
	i32_x4 Result = (I32X4(0xFF) << 24) | (I32X4(Color.r) << 16) | (I32X4(Color.g) << 8) | I32X4(Color.b);
	return Result;
}


void DrawTriangle(clip_vertex Vertex0, clip_vertex Vertex1, clip_vertex Vertex2,
    texture Texture, sampler Sampler)
{

    Vertex0.Pos.w = 1.0f / Vertex0.Pos.w;
    Vertex1.Pos.w = 1.0f / Vertex1.Pos.w;
    Vertex2.Pos.w = 1.0f / Vertex2.Pos.w;

    Vertex0.Pos.xyz *= Vertex0.Pos.w;
    Vertex1.Pos.xyz *= Vertex1.Pos.w;
    Vertex2.Pos.xyz *= Vertex2.Pos.w;

    Vertex0.Uv *= Vertex0.Pos.w;
    Vertex1.Uv *= Vertex1.Pos.w;
    Vertex2.Uv *= Vertex2.Pos.w;

    v2 PointAF = NdcToPixels(Vertex0.Pos.xy);
    v2 PointBF = NdcToPixels(Vertex1.Pos.xy);
    v2 PointCF = NdcToPixels(Vertex2.Pos.xy);

    i32 MinX = min(min((i32)PointAF.x, (i32)PointBF.x), (i32)PointCF.x);
    i32 MaxX = max(max((i32)round(PointAF.x), (i32)round(PointBF.x)), (i32)round(PointCF.x));
    i32 MinY = min(min((i32)PointAF.y, (i32)PointBF.y), (i32)PointCF.y);
    i32 MaxY = max(max((i32)round(PointAF.y), (i32)round(PointBF.y)), (i32)round(PointCF.y));

#if 1
    MinX = max(0, MinX);
    MinX = min(GlobalState.FrameBufferWidth - 1, MinX);
    MaxX = max(0, MaxX);
    MaxX = min(GlobalState.FrameBufferWidth - 1, MaxX);
#endif

    v2i PointA = V2I_F24_8(PointAF);
    v2i PointB = V2I_F24_8(PointBF);
    v2i PointC = V2I_F24_8(PointCF);

    v2i Edge0 = PointB - PointA;
    v2i Edge1 = PointC - PointB;
    v2i Edge2 = PointA - PointC;

    b32 IsTopLeft0 = (Edge0.y > 0.) || (Edge0.x > 0 && Edge0.y == 0);
    b32 IsTopLeft1 = (Edge1.y > 0) || (Edge1.x > 0 && Edge1.y == 0);
    b32 IsTopLeft2 = (Edge2.y > 0) || (Edge2.x > 0 && Edge2.y == 0);

    f32_x4 BaryCentricDiv = F32X4(256.0f / (f32)(CrossProduct2d(PointB - PointA, PointC - PointA)));

    i32_x4 Edge0DiffX = I32X4(Edge0.y);
    i32_x4 Edge1DiffX = I32X4(Edge1.y);
    i32_x4 Edge2DiffX = I32X4(Edge2.y);

    i32_x4 Edge0DiffY = I32X4 (- Edge0.x);
    i32_x4 Edge1DiffY = I32X4 (- Edge1.x);
    i32_x4 Edge2DiffY = I32X4 (- Edge2.x);

    i32_x4 Edge0RowY = {};
    i32_x4 Edge1RowY = {};
    i32_x4 Edge2RowY = {};

    {
        v2i StartPos = V2I_F24_8(V2(MinX, MinY) + V2(0.5f, 0.5f));
        i64 Edge0RowY64 = CrossProduct2d(StartPos - PointA, Edge0);
        i64 Edge1RowY64 = CrossProduct2d(StartPos - PointB, Edge1);
        i64 Edge2RowY64 = CrossProduct2d(StartPos - PointC, Edge2);

        i32 Edge0RowY32 = i32((Edge0RowY64 + Sign(Edge0RowY64) * 128) / 256) - (IsTopLeft0 ? 0 : -1);
        i32 Edge1RowY32 = i32((Edge1RowY64 + Sign(Edge1RowY64) * 128) / 256) - (IsTopLeft1 ? 0 : -1);
        i32 Edge2RowY32 = i32((Edge2RowY64 + Sign(Edge2RowY64) * 128) / 256) - (IsTopLeft2 ? 0 : -1);

        Edge0RowY = I32X4(Edge0RowY32) + I32X4(0, 1, 2, 3) * Edge0DiffX;
        Edge1RowY = I32X4(Edge1RowY32) + I32X4(0, 1, 2, 3) * Edge1DiffX;
        Edge2RowY = I32X4(Edge2RowY32) + I32X4(0, 1, 2, 3) * Edge2DiffX;
    }

    Edge0DiffX = Edge0DiffX * I32X4(4);
    Edge1DiffX = Edge1DiffX * I32X4(4);
    Edge2DiffX = Edge2DiffX * I32X4(4);

    for (i32 Y = MinY; Y <= MaxY; ++Y)
    {
        i32_x4 Edge0RowX = Edge0RowY;
        i32_x4 Edge1RowX = Edge1RowY;
        i32_x4 Edge2RowX = Edge2RowY;

        for (i32 X = MinX; X <= MaxX; X += 4)
        {
            /*if ((Edge0RowX > 0 || (IsTopLeft0 && Edge0RowX == 0)) &&
                (Edge1RowX > 0 || (IsTopLeft1 && Edge1RowX == 0)) &&
                (Edge2RowX > 0 || (IsTopLeft2 && Edge2RowX == 0)))
            {*/
            u32 PixelId = Y * GlobalState.FrameBufferStride + X;
            i32* ColorPtr = (i32*)GlobalState.FrameBufferPixels + PixelId;
            f32* DepthPtr = GlobalState.DepthBuffer + PixelId;
            i32_x4 PixelColors = I32X4Load(ColorPtr);
            f32_x4 PixelDepths = F32X4Load(DepthPtr);

            i32_x4 EdgeMask = (Edge0RowX | Edge1RowX | Edge2RowX) >= 0;

            if (_mm_movemask_epi8(EdgeMask.Vals))
            {
                // NOTE: Ми у середині трикутника

                f32_x4 T0 = -F32X4(Edge1RowX) * BaryCentricDiv;
                f32_x4 T1 = -F32X4(Edge2RowX) * BaryCentricDiv;
                f32_x4 T2 = -F32X4(Edge0RowX) * BaryCentricDiv;

                f32_x4 DepthZ = T0 * Vertex0.Pos.z + T1 * Vertex1.Pos.z + T2 * Vertex2.Pos.z;

                i32_x4 DepthMask = I32X4ReInterpret(DepthZ < PixelDepths);

                f32_x4 OneOverW = T0 * Vertex0.Pos.w + T1 * Vertex1.Pos.w + T2 * Vertex2.Pos.w;

                v2_x4 Uv = T0 * Vertex0.Uv + T1 * Vertex1.Uv + T2 * Vertex2.Uv;
                Uv = Uv / OneOverW;

                i32_x4 TexelColor = I32X4(0);
                switch (Sampler.Type)
                {
                case SamplerType_Nearest:
                {
                    i32_x4 TexelX = I32X4(Floor(Uv.x * (Texture.Width - 1)));
                    i32_x4 TexelY = I32X4(Floor(Uv.y * (Texture.Height - 1)));

                    i32_x4 TexelMask = (TexelX >= 0 & TexelX < Texture.Width &
                        TexelY >= 0 & TexelY < Texture.Height);

                    TexelX = Max(Min(TexelX, Texture.Width - 1), 0);
                    TexelY = Max(Min(TexelY, Texture.Height - 1), 0);

                    i32_x4 TexelOffsets = TexelY * Texture.Width + TexelX;

                    i32_x4 TrueCase = I32X4Gather((i32*)Texture.Texels, TexelOffsets);
                    i32_x4 FalseCase = I32X4(0xFF00FF00);

                    TexelColor = (TrueCase & TexelMask) + AndNot(TexelMask, FalseCase);
                } break;

                case SamplerType_Bilinear:
                {
                    v2_x4 TexelV2 = Uv * V2(Texture.Width, Texture.Height) - V2(0.5f, 0.5f);
                    v2i_x4 TexelPos[4] = {};
                    TexelPos[0] = V2IX4(Floor(TexelV2.x), Floor(TexelV2.y));
                    TexelPos[1] = TexelPos[0] + V2I(1, 0);
                    TexelPos[2] = TexelPos[0] + V2I(0, 1);
                    TexelPos[3] = TexelPos[0] + V2I(1, 1);

                    v3_x4 TexelColors[4] = {};
                    for (u32 TexelId = 0; TexelId < ArrayCount(TexelPos); ++TexelId)
                    {
                        v2i_x4 CurrTexelPos = TexelPos[TexelId];
                        i32_x4 TexelMask = (CurrTexelPos.x >= 0 & CurrTexelPos.x < Texture.Width &
                            CurrTexelPos.y >= 0 & CurrTexelPos.y < Texture.Height);

                        CurrTexelPos.x = Max(Min(CurrTexelPos.x, Texture.Width - 1), 0);
                        CurrTexelPos.y = Max(Min(CurrTexelPos.y, Texture.Height - 1), 0);
                        i32_x4 TexelOffsets = CurrTexelPos.y * Texture.Width + CurrTexelPos.x;

                        i32_x4 TrueCase = I32X4Gather((i32*)Texture.Texels, TexelOffsets);
                        i32_x4 FalseCase = I32X4(Sampler.BorderColor);
                        i32_x4 TexelColorI32 = (TrueCase & TexelMask) + AndNot(TexelMask, FalseCase);

                        TexelColors[TexelId] = ColorI32ToRgb(TexelColorI32);
                    }

                    f32_x4 S = TexelV2.x - Floor(TexelV2.x);
                    f32_x4 K = TexelV2.y - Floor(TexelV2.y);

                    v3_x4 Interpolated0 = Lerp(TexelColors[0], TexelColors[1], S);
                    v3_x4 Interpolated1 = Lerp(TexelColors[2], TexelColors[3], S);
                    v3_x4 FinalColor = Lerp(Interpolated0, Interpolated1, K);

                    TexelColor = ColorRgbToI32(FinalColor);
                } break;

                default:
                {
                    InvalidCodePath;
                }
                }

                i32_x4 FinalMaskI32 = EdgeMask & DepthMask;
                f32_x4 FinalMaskF32 = F32X4ReInterpret(FinalMaskI32);
                i32_x4 OutputColors = (TexelColor & FinalMaskI32) + AndNot(FinalMaskI32, PixelColors);
                f32_x4 OutputDepths = (DepthZ & FinalMaskF32) + AndNot(FinalMaskF32, PixelDepths);
                I32X4Store(ColorPtr, OutputColors);
                F32X4Store(DepthPtr, OutputDepths);
            }

            Edge0RowX += Edge0DiffX;
            Edge1RowX += Edge1DiffX;
            Edge2RowX += Edge2DiffX;
        }

        Edge0RowY += Edge0DiffY;
        Edge1RowY += Edge1DiffY;
        Edge2RowY += Edge2DiffY;
    }

}

void DrawTriangle(v4 ModelVertex0, v4 ModelVertex1, v4 ModelVertex2,
    v2 ModelUv0, v2 ModelUv1, v2 ModelUv2,
    texture Texture, sampler Sampler)
{
    clip_result Ping = {};
    Ping.NumTriangles = 1;
    Ping.Vertices[0] = { ModelVertex0, ModelUv0 };
    Ping.Vertices[1] = { ModelVertex1, ModelUv1 };
    Ping.Vertices[2] = { ModelVertex2, ModelUv2 };

    clip_result Pong = {};

    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Left);
    ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Right);
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Top);
    ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Bottom);
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_Near);
    ClipPolygonToAxis(&Pong, &Ping, ClipAxis_Far);
    ClipPolygonToAxis(&Ping, &Pong, ClipAxis_W);

    for (u32 TriangleId = 0; TriangleId < Pong.NumTriangles; ++TriangleId)
    {
        DrawTriangle(Pong.Vertices[3 * TriangleId + 0], Pong.Vertices[3 * TriangleId + 1],
            Pong.Vertices[3 * TriangleId + 2], Texture, Sampler);
    }
}

LRESULT Win32WindowCallBack(HWND WindowHandle,
							UINT Message,
							WPARAM WParam,
							LPARAM LParam)
{
	LRESULT Result = {};

	switch (Message)
	{
		case WM_CLOSE:
		case WM_DESTROY:
		{
			GlobalState.IsRunning = false;
		}break;

		default:
		{
			Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
		}break;
	}
		
	return Result;
}

int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd)
{
    GlobalState.IsRunning = true;
    LARGE_INTEGER TimerFrequency = {};
    Assert(QueryPerformanceFrequency(&TimerFrequency));

    {
        WNDCLASSA WindowClass = {};
        WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        WindowClass.lpfnWndProc = Win32WindowCallBack;
        WindowClass.hInstance = hInstance;
        WindowClass.hCursor = LoadCursorA(NULL, MAKEINTRESOURCEA(32512));
        WindowClass.lpszClassName = "Graphics Tutorial";
        if (!RegisterClassA(&WindowClass))
        {
            InvalidCodePath;
        }

        GlobalState.WindowHandle = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "Graphics",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            1280,
            720,
            NULL,
            NULL,
            hInstance,
            NULL);

        if (!GlobalState.WindowHandle)
        {
            InvalidCodePath;
        }

        GlobalState.DeviceContext = GetDC(GlobalState.WindowHandle);
    }

    {
        RECT ClientRect = {};
        Assert(GetClientRect(GlobalState.WindowHandle, &ClientRect));
        GlobalState.FrameBufferWidth = ClientRect.right - ClientRect.left;
        GlobalState.FrameBufferHeight = ClientRect.bottom - ClientRect.top;
 
        GlobalState.FrameBufferWidth = 600;
        GlobalState.FrameBufferStride = GlobalState.FrameBufferWidth + 3;
        GlobalState.FrameBufferHeight = 600;
        GlobalState.FrameBufferPixels = (u32*)malloc(sizeof(u32) * GlobalState.FrameBufferStride *
            GlobalState.FrameBufferHeight);
        GlobalState.DepthBuffer = (f32*)malloc(sizeof(f32) * GlobalState.FrameBufferStride *
            GlobalState.FrameBufferHeight);
    }

    texture CheckerBoardTexture = {};
    sampler Sampler = {};
    {
        Sampler.Type = SamplerType_Bilinear;
        Sampler.BorderColor = 0xFF000000;

        u32 BlockSize = 4;
        u32 NumBlocks = 8;

        CheckerBoardTexture.Width = BlockSize * NumBlocks;
        CheckerBoardTexture.Height = BlockSize * NumBlocks;
        CheckerBoardTexture.Texels = (u32*)malloc(sizeof(u32) * CheckerBoardTexture.Width * CheckerBoardTexture.Height);
        for (u32 Y = 0; Y < NumBlocks; ++Y)
        {
            for (u32 X = 0; X < NumBlocks; ++X)
            {
                u32 ColorChannel = 255 * ((X + (Y % 2)) % 2);

                for (u32 BlockY = 0; BlockY < BlockSize; ++BlockY)
                {
                    for (u32 BlockX = 0; BlockX < BlockSize; ++BlockX)
                    {
                        u32 TexelId = (Y * BlockSize + BlockY) * CheckerBoardTexture.Width + (X * BlockSize + BlockX);
                        CheckerBoardTexture.Texels[TexelId] = ((u32)0xFF << 24) | ((u32)ColorChannel << 16) | ((u32)ColorChannel << 8) | (u32)ColorChannel;
                    }
                }
            }
        }
    }

    LARGE_INTEGER BeginTime = {};
    LARGE_INTEGER EndTime = {};
    Assert(QueryPerformanceCounter(&BeginTime));

    while (GlobalState.IsRunning)
    {
        Assert(QueryPerformanceCounter(&EndTime));
        f32 FrameTime = f32(EndTime.QuadPart - BeginTime.QuadPart) / f32(TimerFrequency.QuadPart);
        BeginTime = EndTime;

        {
            char Text[256];
            snprintf(Text, sizeof(Text), "FrameTime %f\n", FrameTime);
            OutputDebugStringA(Text);
        }

        MSG Message = {};
        while (PeekMessageA(&Message, GlobalState.WindowHandle, 0, 0, PM_REMOVE))
        {
            switch (Message.message)
            {
            case WM_QUIT:
            {
                GlobalState.IsRunning = false;
            } break;

            case WM_KEYUP:
            case WM_KEYDOWN:
            {
                u32 VkCode = Message.wParam;
                b32 IsDown = !((Message.lParam >> 31) & 0x1);

                switch (VkCode)
                {
                case 'W':
                {
                    GlobalState.WDown = IsDown;
                } break;

                case 'A':
                {
                    GlobalState.ADown = IsDown;
                } break;

                case 'S':
                {
                    GlobalState.SDown = IsDown;
                } break;

                case 'D':
                {
                    GlobalState.DDown = IsDown;
                } break;
                }
            } break;

            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
            }
        }

        // NOTE: Очистуємо буфер кадри до 0
        for (u32 Y = 0; Y < GlobalState.FrameBufferHeight; ++Y)
        {
            for (u32 X = 0; X < GlobalState.FrameBufferWidth; ++X)
            {
                u32 PixelId = Y * GlobalState.FrameBufferStride + X;

                u8 Red = (u8)0;
                u8 Green = (u8)0;
                u8 Blue = 0;
                u8 Alpha = 255;
                u32 PixelColor = ((u32)Alpha << 24) | ((u32)Red << 16) | ((u32)Green << 8) | (u32)Blue;

                GlobalState.DepthBuffer[PixelId] = FLT_MAX;
                GlobalState.FrameBufferPixels[PixelId] = PixelColor;
            }
        }

        RECT ClientRect = {};
        Assert(GetClientRect(GlobalState.WindowHandle, &ClientRect));
        u32 ClientWidth = ClientRect.right - ClientRect.left;
        u32 ClientHeight = ClientRect.bottom - ClientRect.top;
        f32 AspectRatio = f32(ClientWidth) / f32(ClientHeight);

        // NOTE: Обчислюємо положення камери
        m4 CameraTransform = IdentityM4();
        {
            camera* Camera = &GlobalState.Camera;

            b32 MouseDown = false;
            v2 CurrMousePos = {};
            if (GetActiveWindow() == GlobalState.WindowHandle)
            {
                POINT Win32MousePos = {};
                Assert(GetCursorPos(&Win32MousePos));
                Assert(ScreenToClient(GlobalState.WindowHandle, &Win32MousePos));

                Win32MousePos.y = ClientRect.bottom - Win32MousePos.y;

                CurrMousePos.x = f32(Win32MousePos.x) / f32(ClientWidth);
                CurrMousePos.y = f32(Win32MousePos.y) / f32(ClientHeight);

                MouseDown = (GetKeyState(VK_LBUTTON) & 0x80) != 0;
            }

            if (MouseDown)
            {
                if (!Camera->PrevMouseDown)
                {
                    Camera->PrevMousePos = CurrMousePos;
                }

                v2 MouseDelta = CurrMousePos - Camera->PrevMousePos;
                Camera->Pitch += MouseDelta.y;
                Camera->Yaw += MouseDelta.x;

                Camera->PrevMousePos = CurrMousePos;
            }

            Camera->PrevMouseDown = MouseDown;

            m4 YawTransform = RotationMatrix(0, Camera->Yaw, 0);
            m4 PitchTransform = RotationMatrix(Camera->Pitch, 0, 0);
            m4 CameraAxisTransform = YawTransform * PitchTransform;

            v3 Right = Normalize((CameraAxisTransform * V4(1, 0, 0, 0)).xyz);
            v3 Up = Normalize((CameraAxisTransform * V4(0, 1, 0, 0)).xyz);
            v3 LookAt = Normalize((CameraAxisTransform * V4(0, 0, 1, 0)).xyz);

            m4 CameraViewTransform = IdentityM4();

            CameraViewTransform.v[0].x = Right.x;
            CameraViewTransform.v[1].x = Right.y;
            CameraViewTransform.v[2].x = Right.z;

            CameraViewTransform.v[0].y = Up.x;
            CameraViewTransform.v[1].y = Up.y;
            CameraViewTransform.v[2].y = Up.z;

            CameraViewTransform.v[0].z = LookAt.x;
            CameraViewTransform.v[1].z = LookAt.y;
            CameraViewTransform.v[2].z = LookAt.z;

            if (GlobalState.WDown)
            {
                Camera->Pos += LookAt * FrameTime;
            }
            if (GlobalState.SDown)
            {
                Camera->Pos -= LookAt * FrameTime;
            }
            if (GlobalState.DDown)
            {
                Camera->Pos += Right * FrameTime;
            }
            if (GlobalState.ADown)
            {
                Camera->Pos -= Right * FrameTime;
            }

            CameraTransform = CameraViewTransform * TranslationMatrix(-Camera->Pos);
        }

        // NOTE: Проєктуємо наші трикутники
        GlobalState.CurrTime = GlobalState.CurrTime + FrameTime;
        if (GlobalState.CurrTime > 2.0f * 3.14159f)
        {
            GlobalState.CurrTime -= 2.0f * 3.14159f;
        }

        //GlobalState.CurrTime = 0;

        v3 ModelVertices[] =
        {
            // NOTE: Front Face
            V3(-0.5f, -0.5f, -0.5f),
            V3(-0.5f, 0.5f, -0.5f),
            V3(0.5f, 0.5f, -0.5f),
            V3(0.5f, -0.5f, -0.5f),

            // NOTE: Back Face
            V3(-0.5f, -0.5f, 0.5f),
            V3(-0.5f, 0.5f, 0.5f),
            V3(0.5f, 0.5f, 0.5f),
            V3(0.5f, -0.5f, 0.5f),
        };

        v2 ModelUvs[] =
        {
            V2(0, 0),
            V2(1, 0),
            V2(1, 1),
            V2(0, 1),

            V2(0, 0),
            V2(1, 0),
            V2(1, 1),
            V2(0, 1),
        };

        u32 ModelIndices[] =
        {
            // NOTE: Front Face
            0, 1, 2,
            2, 3, 0,

            // NOTE: Back Face
            6, 5, 4,
            4, 7, 6,

            // NOTE: Left face
            4, 5, 1,
            1, 0, 4,

            // NOTE: Right face
            3, 2, 6,
            6, 7, 3,

            // NOTE: Top face
            1, 5, 6,
            6, 2, 1,

            // NOTE: Bottom face
            4, 0, 3,
            3, 7, 4,
        };

        f32 Offset = abs(sin(GlobalState.CurrTime));
        m4 Transform = (PerspectiveMatrix(60.0f, AspectRatio, 0.01f, 1000.0f) *
            CameraTransform *
            TranslationMatrix(0, 0, 2) *
            RotationMatrix(0, GlobalState.CurrTime, 0) *
            ScaleMatrix(1, 1, 1));

        v4* TransformedVertices = (v4*)malloc(sizeof(v4) * ArrayCount(ModelVertices));
        for (u32 VertexId = 0; VertexId < ArrayCount(ModelVertices); ++VertexId)
        {
            TransformedVertices[VertexId] = (Transform * V4(ModelVertices[VertexId], 1.0f));
        }

        for (u32 IndexId = 0; IndexId < ArrayCount(ModelIndices); IndexId += 3)
        {
            u32 Index0 = ModelIndices[IndexId + 0];
            u32 Index1 = ModelIndices[IndexId + 1];
            u32 Index2 = ModelIndices[IndexId + 2];

            DrawTriangle(TransformedVertices[Index0], TransformedVertices[Index1], TransformedVertices[Index2],
                ModelUvs[Index0], ModelUvs[Index1], ModelUvs[Index2],
                CheckerBoardTexture, Sampler);
        }

        free(TransformedVertices);

        

        BITMAPINFO BitmapInfo = {};
        BitmapInfo.bmiHeader.biSize = sizeof(tagBITMAPINFOHEADER);
        BitmapInfo.bmiHeader.biWidth = GlobalState.FrameBufferStride;
        BitmapInfo.bmiHeader.biHeight = GlobalState.FrameBufferHeight;
        BitmapInfo.bmiHeader.biPlanes = 1;
        BitmapInfo.bmiHeader.biBitCount = 32;
        BitmapInfo.bmiHeader.biCompression = BI_RGB;

        Assert(StretchDIBits(GlobalState.DeviceContext,
            0,
            0,
            ClientWidth,
            ClientHeight,
            0,
            0,
            GlobalState.FrameBufferWidth,
            GlobalState.FrameBufferHeight,
            GlobalState.FrameBufferPixels,
            &BitmapInfo,
            DIB_RGB_COLORS,
            SRCCOPY));
    }



    return 0;
}

// MAKEINTRESOURCEA(32512)