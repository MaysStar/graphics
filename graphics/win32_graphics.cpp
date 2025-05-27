#include <cmath>
#include <Windows.h>

#include "win32_graphics.h"

global f32 Pi32 = 3.14159265359;
global global_state GlobalState;

v2 ProjectPoint(v3 Pos)
{
	v2 Result = Pos.xy / Pos.z;
	Result = 0.5f * (Result + V2(1.0f)) * V2((f32)GlobalState.FrameBufferWidth, (f32)GlobalState.FrameBufferHeight);
	return Result;
}

f32 CrossProduct2d(v2 A, v2 B)
{
	f32 Result = A.x * B.y - A.y * B.x;
	return Result;
}

void  DrawTriangle(v3* Points, v3* Colors)
{
	v2 PointA = ProjectPoint(Points[0]);
	v2 PointB = ProjectPoint(Points[1]);
	v2 PointC = ProjectPoint(Points[2]);

	i32 MinX = min(min((i32)PointA.x, (i32)PointB.x), (i32)PointC.x);
	i32 MaxX = max(max((i32)round(PointA.x), (i32)round(PointB.x)), (i32)round(PointC.x));
	i32 MinY = min(min((i32)PointA.y, (i32)PointB.y), (i32)PointC.y);
	i32 MaxY = max(max((i32)round(PointA.y), (i32)round(PointB.y)), (i32)round(PointC.y));

	MinX = max(0, MinX);
	MinX = min(GlobalState.FrameBufferWidth - 1, MinX);
	MaxX = max(0, MaxX);
	MaxX = min(GlobalState.FrameBufferWidth - 1, MaxX);

	MinY = max(0, MinY);
	MinY = min(GlobalState.FrameBufferHeight - 1, MinY);
	MaxY = max(0, MaxY);
	MaxY = min(GlobalState.FrameBufferHeight - 1, MaxY);

	v2 Edge0 = PointB - PointA;
	v2 Edge1 = PointC - PointB;
	v2 Edge2 = PointA - PointC;

	b32 isTopLeft0 = (Edge0.x >= 0.0f && Edge0.y > 0.0f) || (Edge0.x > 0.0f && Edge0.y == 0.0f);
	b32 isTopLeft1 = (Edge1.x >= 0.0f && Edge1.y > 0.0f) || (Edge1.x > 0.0f && Edge1.y == 0.0f);
	b32 isTopLeft2 = (Edge2.x >= 0.0f && Edge2.y > 0.0f) || (Edge2.x > 0.0f && Edge2.y == 0.0f);

	f32 BaryCenrticDiv = CrossProduct2d(PointB - PointA, PointC - PointA);

	for (i32 Y = MinY; Y <= MaxY;  Y++)
	{
		for (i32 X = MinX; X <= MaxX; X++)
		{
			v2 PixelPoint = V2(X, Y) + V2(0.5f);

			v2 PixelEdge0 = PixelPoint - PointA;
			v2 PixelEdge1 = PixelPoint - PointB;
			v2 PixelEdge2 = PixelPoint - PointC;

			f32 CrossLength0 = CrossProduct2d(PixelEdge0, Edge0);
			f32 CrossLength1 = CrossProduct2d(PixelEdge1, Edge1);
			f32 CrossLength2 = CrossProduct2d(PixelEdge2, Edge2);

			if ((CrossLength0 > 0.0f || (isTopLeft0 && CrossLength0 == 0.0)) &&
				(CrossLength1 > 0.0f || (isTopLeft1 && CrossLength1 == 0.0)) &&
				(CrossLength2 > 0.0f || (isTopLeft2 && CrossLength2 == 0.0)))
			{
				// ми всередині трикутника
				u32 PixelId = Y * GlobalState.FrameBufferWidth + X;

				f32 T0 = -CrossLength1 / BaryCenrticDiv;
				f32 T1 = -CrossLength2 / BaryCenrticDiv;
				f32 T2 = -CrossLength0 / BaryCenrticDiv;

				f32 Depth = T0 * ( 1.0f / Points[0].z) + T1 * (1.0f / Points[1].z) + T2 * (1.0f / Points[2].z);
				Depth = 1.0f / Depth;

				if (Depth < GlobalState.DepthBuffer[PixelId])
				{
					v3 FinalColor = T0 * Colors[0] + T1 * Colors[1] + T2 * Colors[2];

					FinalColor = 255.0f * FinalColor;

					u32 FinalColorU32 = ((u32)0xFF << 24) | ((u32)FinalColor.r << 16) | ((u32)FinalColor.g << 8) | (u32)FinalColor.b;

					GlobalState.FrameBufferPixels[PixelId] = FinalColorU32;

					GlobalState.DepthBuffer[PixelId] = Depth;
				}
			}
		}
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

 int APIENTRY __stdcall WinMain(HINSTANCE hInstance,
								HINSTANCE hPrevInstance,
								LPSTR lpCmdLine,
								int nShowCmd)
{
	GlobalState.IsRunning = true;
	LARGE_INTEGER TimerFrequency = {};
	Assert(QueryPerformanceFrequency(&TimerFrequency));

	{
		WNDCLASSA WindowClass = {};
		WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		WindowClass.lpfnWndProc = Win32WindowCallBack;
		WindowClass.cbClsExtra = 0;
		WindowClass.cbWndExtra = 0;
		WindowClass.hInstance = hInstance;
		WindowClass.hIcon = 0;
		WindowClass.hCursor = LoadCursorA(NULL, MAKEINTRESOURCEA(32512)); // 32649 corresponds to IDC_HAND in ANSI //IDC_ARROW 32512
		WindowClass.hbrBackground = 0;
		WindowClass.lpszMenuName = 0;
		WindowClass.lpszClassName = "Graphics";
		if (!RegisterClassA(&WindowClass))
		{
			InvalidCodePath;
		}

		GlobalState.WindowHandle = CreateWindowExA(
			0,
			WindowClass.lpszClassName,
			"GAME",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			1280,
			720,
			NULL,
			NULL,
			hInstance,
			NULL
		);

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
		//GlobalState.FrameBufferWidth = 300;
		//GlobalState.FrameBufferHeight = 300;
		GlobalState.FrameBufferPixels = (u32*)malloc(sizeof(u32) * GlobalState.FrameBufferWidth
			* GlobalState.FrameBufferHeight);
		GlobalState.DepthBuffer = (f32*)malloc(sizeof(f32) * GlobalState.FrameBufferWidth
			* GlobalState.FrameBufferHeight);
	}

	LARGE_INTEGER BeginTime = {};
	LARGE_INTEGER EndTime = {};
	Assert(QueryPerformanceCounter(&BeginTime));

	while (GlobalState.IsRunning)
	{
		Assert(QueryPerformanceCounter(&EndTime));
		f32 FrameTime = (f32)(EndTime.QuadPart - BeginTime.QuadPart) / (f32)(TimerFrequency.QuadPart);
		BeginTime = EndTime;

		MSG Message = {};
		while (PeekMessageA(&Message, GlobalState.WindowHandle, 0, 0, PM_REMOVE))
		{
			switch (Message.message)
			{
				case WM_QUIT:
				{
					GlobalState.IsRunning = false;
				}break;

				default:
				{
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}break;
			}
		}

		f32 Speed = 150.0f;
		GlobalState.CursorOffset += Speed * FrameTime;

		for (u32 Y = 0; Y < GlobalState.FrameBufferHeight; Y++)
		{
			for (u32 X = 0; X < GlobalState.FrameBufferWidth; X++)
			{
				u32 PixelId = Y * GlobalState.FrameBufferWidth + X;

				u8 Red = 0;
				u8 Green = 0;
				u8 Blue = 0;
				u8 Alpha = 255;

				u32 PixelColor = ((u32)Alpha << 24) | ((u32)Red << 16) | ((u32)Green << 8) | (u32)Blue;

				GlobalState.DepthBuffer[PixelId] = FLT_MAX;
				GlobalState.FrameBufferPixels[PixelId] = PixelColor;
			}
		}

		v3 Colors1[3]
		{
			V3(1, 0, 0),
			V3(0, 1, 0),
			V3(0, 0, 1),
		};

		v3 Colors2[3]
		{
			V3(0, 1, 1),
			V3(1, 1, 0),
			V3(1, 1, 1),
		};

		v3 Points1[3]
		{
			V3(0.0f, 0.5f, 1.0f),
			V3(0.5f, -0.5f, 1.0f),
			V3(-0.5f, -0.5f, 1.0f),
		};

		v3 Points2[3]
		{
			V3(0.0f, 0.5f, 1.2f),
			V3(0.5f, -0.5f, 1.2f),
			V3(-0.5f, -0.5f, 0.8f),
		};

		/*DrawTriangle(Points1, Colors1);
		DrawTriangle(Points2, Colors2);*/

#if 1
		for (i32 TriangleId = 0; TriangleId < 9; TriangleId++) {
			f32 Depth = powf(2.0f, (f32)(TriangleId + 1));

			v3 Points[3] =
			{
				V3(-1.0f, -0.5f, Depth),
				V3(0.0f, 0.5f, Depth + 1.0f),
				V3(1.0f, -0.5f, Depth - 0.5f),
			};

			for (u32 PixelId = 0; PixelId < ArrayCount(Points); PixelId++)
			{
				v3 TransformedPos = Points[PixelId] + V3(cosf(GlobalState.CurrAngle), sinf(GlobalState.CurrAngle), 0);
				/*v2 PixelPos = ProjectPoint(TransformedPos);

				if (PixelPos.x >= 0.0f && PixelPos.x < GlobalState.FrameBufferWidth &&
					PixelPos.y >= 0.0f && PixelPos.y < GlobalState.FrameBufferHeight)
				{
					u32 PixelId = (u32)(PixelPos.y) * GlobalState.FrameBufferWidth + (u32)(PixelPos.x);
					GlobalState.FrameBufferPixels[PixelId] = 0xFF00FF00;
				}*/
				Points[PixelId] = TransformedPos;
			}

			DrawTriangle(Points, Colors1);
		}
#endif
		GlobalState.CurrAngle += FrameTime;
		if (GlobalState.CurrAngle >= 2.0f * Pi32)
		{
			GlobalState.CurrAngle -= 2.0f * Pi32;
		}

		RECT ClientRect = {};
		Assert(GetClientRect(GlobalState.WindowHandle, &ClientRect));
		u32 ClientWidth = ClientRect.right - ClientRect.left;
		u32 ClientHeight = ClientRect.bottom - ClientRect.top;

		BITMAPINFO BitmapInfo = {};
		BitmapInfo.bmiHeader.biSize = sizeof(tagBITMAPINFOHEADER);
		BitmapInfo.bmiHeader.biWidth = GlobalState.FrameBufferWidth;
		BitmapInfo.bmiHeader.biHeight = GlobalState.FrameBufferHeight;
		BitmapInfo.bmiHeader.biPlanes = 1;
		BitmapInfo.bmiHeader.biBitCount = 32;
		BitmapInfo.bmiHeader.biCompression = BI_RGB;

		Assert(StretchDIBits(
			GlobalState.DeviceContext,
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
			SRCCOPY
		));
	}

	delete GlobalState.FrameBufferPixels;

	return 0;
}