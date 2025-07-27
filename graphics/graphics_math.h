#if !defined(GRAPHICS_MATH_H)

#include "base_def.h"

union i32_x4
{
	struct
	{
		__m128i Vals;
	};
	
	i32 e[4];
};

struct f32_x4
{
	__m128 Vals;
};

union v2
{
	struct 
	{
		f32 x, y;
	};

	f32 e[2];
};

union v2_x4
{
	struct
	{
		f32_x4 x, y;
	};

	f32_x4 e[2];
};

union v2i
{
	struct
	{
		i32 x, y;
	};

	i32 e[2];
};

union v2i_x4
{
	struct
	{
		i32_x4 x, y;
	};

	i32_x4 e[2];
};

union v3 
{
	struct
	{
		f32 x, y, z;
	};

	struct
	{
		f32 r, g, b;
	};

	struct
	{
		v2 xy;
		f32 Ignored0;
	};

	f32 e[3];
};

union v3_x4
{
	struct
	{
		f32_x4 x, y, z;
	};

	struct
	{
		f32_x4 r, g, b;
	};

	struct
	{
		v2_x4 xy;
		f32_x4 Ignored0;
	};

	struct
	{
		f32_x4 Ignored1;
		v2_x4 yz;
	};

	f32_x4 e[3];
};

union v4
{
	struct
	{
		f32 x, y, z, w;
	};

	struct
	{
		f32 r, g, b, a;
	};

	struct
	{
		v3 xyz;
		f32 Ignored0;
	};

	struct
	{
		v2 xy;
		f32 Ignored0;
	};

	f32 e[4];
};

union m4
{
	v4 v[4];
	f32 e[16];
};

v2 V2(f32);
v2 V2(f32, f32);

v2 operator+(v2, v2);
v2 operator-(v2, v2);
v2 operator*(v2, v2);
v2 operator*(f32, v2);
v2 operator/(v2, f32);
v2 operator/=(v2&, f32);
v2 operator*=(v2&, f32);

v2i V2I(f32, f32);
v2i V2I_F24_8(v2);
v2i operator+(v2i, v2i);
v2i operator-(v2i, v2i);

i64 Sign(i64);

v3 V3(f32, f32, f32);
v3 operator+(v3, v3);
v3 operator-(v3, v3);
v3 operator*(f32, v3);
v3 operator*(v3, f32);
v3 operator/(v3, f32);
v3 Normalize(v3 A);
v3 operator+=(v3&, v3);
v3 operator-=(v3&, v3);
v3 operator-(v3);
v3 operator/=(v3&, f32);
v3 operator*=(v3&, f32);
v3 Lerp(v3, v3, f32);

v4 V4(f32, f32, f32, f32);
v4 V4(v3, f32);
v4 operator+(v4, v4);
v4 operator*(v4, v4);
v4 operator*(v4, f32);
v4 operator*(f32, v4);
v4 operator*(m4, v4);

m4 IdentityM4();
m4 operator*(m4, m4);
m4 ScaleMatrix(f32, f32, f32);
m4 TranslationMatrix(v3);
m4 TranslationMatrix(f32, f32, f32);
m4 RotationMatrix(f32, f32, f32);
m4 PerspectiveMatrix(f32, f32, f32, f32);

f32_x4 F32X4(f32);
f32_x4 F32X4(i32_x4);
i32_x4 I32X4(f32_x4);
i32_x4 I32X4(i32);
i32_x4 I32X4(f32, f32, f32, f32);
f32_x4 operator-(f32_x4);
f32_x4 operator+(f32_x4, f32_x4);
f32_x4 operator+(f32_x4, f32_x4);
f32_x4 operator-(f32_x4, f32_x4);
f32_x4 operator-(f32, f32_x4);
f32_x4 operator*(f32_x4, f32_x4);
f32_x4 operator*(f32_x4, f32);
f32_x4 operator<(f32_x4, f32_x4);
f32_x4 operator&(f32_x4, f32_x4);
f32_x4 operator/(f32_x4, f32_x4);
i32_x4 operator+(i32_x4, i32_x4);
i32_x4 operator+=(i32_x4&, i32_x4);
i32_x4 operator-(i32_x4, i32_x4);
i32_x4 operator*(i32_x4, i32_x4);
i32_x4 operator*(i32_x4, i32);
i32_x4 operator|(i32_x4, i32_x4);
i32_x4 operator&(i32_x4, i32_x4);
i32_x4 operator&(i32_x4, i32);
i32_x4 operator>=(i32_x4, i32_x4);
i32_x4 operator>=(i32_x4, i32);
i32_x4 operator<(i32_x4, i32_x4);
i32_x4 operator<<(i32_x4, i32);
i32_x4 operator>>(i32_x4, i32);
i32_x4 operator<(i32_x4, i32);
i32_x4 I32X4Load(i32*);
f32_x4 F32X4Load(f32*);
i32_x4 I32X4ReInterpret(f32_x4);
f32_x4 F32X4ReInterpret(i32_x4);
i32_x4 AndNot(i32_x4, i32_x4);
f32_x4 AndNot(f32_x4, f32_x4);
f32_x4 Floor(f32_x4);
i32_x4 Min(i32_x4, i32_x4);
i32_x4 Min(i32_x4, i32);
i32_x4 Max(i32_x4, i32_x4);
i32_x4 Max(i32_x4, i32);
i32_x4 I32X4Gather(i32*, i32_x4);
void F32X4Store(f32*, f32_x4);
void I32X4Store(i32*, i32_x4);

v2_x4 operator/(v2_x4, f32_x4);
v2_x4 operator*(f32_x4, v2);
v2_x4 operator*(v2_x4, v2);
v2_x4 operator+(v2_x4, v2_x4);
v2_x4 operator-(v2_x4, v2);

v2i_x4 V2IX4(i32, i32);
v2i_x4 V2IX4(f32_x4, f32_x4);
v2i_x4 operator*(v2i_x4, v2);
v2i_x4 operator+(v2i_x4, v2i);

v3_x4 operator/(v3_x4, f32);
v3_x4 operator*(v3_x4, f32);
v3_x4 operator*(v3_x4, f32_x4);
v3_x4 operator*(f32_x4, v3_x4);
v3_x4 operator+(v3_x4, v3_x4);
v3_x4 Lerp(v3_x4, v3_x4, f32_x4);

#define GRAPHICS_MATH_H
#endif