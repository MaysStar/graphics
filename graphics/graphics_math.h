#if !defined(GRAPHICS_MATH_H)

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

	struct
	{
		v2 yz;
		f32 Ignored1;
	};

	f32 e[3];
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

#define GRAPHICS_MATH_H
#endif