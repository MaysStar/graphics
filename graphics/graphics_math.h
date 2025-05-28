#if !defined(GRAPHICS_MATH_H)
#include <cmath>

typedef float f32;

union v2
{
	struct 
	{
		f32 x, y;
	};

	f32 e[2];
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

v3 V3(f32, f32, f32);
v3 operator+(v3, v3);
v3 operator*(f32, v3);

v4 V4(f32, f32, f32, f32);
v4 V4(v3, f32);
v4 operator+(v4, v4);
v4 operator*(v4, v4);
v4 operator*(v4, f32);
v4 operator*(m4, v4);

m4 IdentityM4();
m4 operator*(m4, m4);
m4 ScaleMatrix(f32, f32, f32);
m4 TranslationMatrix(f32, f32, f32);
m4 RotationMatrix(f32, f32, f32);


#define GRAPHICS_MATH_H
#endif