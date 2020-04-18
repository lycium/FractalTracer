#donotrun
/*
Classic Menger sponge fractal.
Folding implementation from Mandelbulber2, credited to knighty.
*/

#define swap(T,a,b) do{ T t = a; a = b; b = t; }while(false)

#define MENGERSPONGE(VEC, REAL, real, SCALAR, scalar) \
void MengerSponge(inout VEC z, SCALAR scale) \
{ \
  /* z = abs(z); // hangs GPU, don't know why */ \
  z.v[0] = lt(z.v[0].x, scalar(0)) ? neg(z.v[0]) : z.v[0]; \
  z.v[1] = lt(z.v[1].x, scalar(0)) ? neg(z.v[1]) : z.v[1]; \
  z.v[2] = lt(z.v[2].x, scalar(0)) ? neg(z.v[2]) : z.v[2]; \
  if (lt(z.v[0].x, z.v[1].x)) swap(REAL, z.v[0], z.v[1]); \
  if (lt(z.v[0].x, z.v[2].x)) swap(REAL, z.v[0], z.v[2]); \
  if (lt(z.v[1].x, z.v[2].x)) swap(REAL, z.v[1], z.v[2]); \
  z = mul(z, real(scale)); \
  z.v[0] = sub(z.v[0], sub(scale, scalar(1))); \
  z.v[1] = sub(z.v[1], sub(scale, scalar(1))); \
  if (lt(scalar(1), z.v[2].x)) \
    z.v[2] = sub(z.v[2], sub(scale, scalar(1))); \
}
MENGERSPONGE(Vec3Dual3f, Dual3f, dual3f, float, float)
MENGERSPONGE(Vec3Dual3fx, Dual3fx, dual3fx, FloatX, floatx)
#undef MENGERSPONGE

#undef swap
