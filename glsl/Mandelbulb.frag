#donotrun
/*
Classic Mandelbulb fractal.
*/

// Trigonometric variant: arbitrary power.
#define MANDELBULB(VEC, REAL, NUM, num) \
void MandelbulbTrig(inout VEC w, in VEC c, NUM n) \
{ \
  REAL xy2 = add(sqr(w.v[0]), sqr(w.v[1])); \
  REAL r2 = add(xy2, sqr(w.v[2])); \
  REAL phi = atan(w.v[1], w.v[0]); \
  REAL theta = atan(sqrt(xy2), w.v[2]); \
  r2 = pow(r2, div(n, num(2))); \
  phi = mul(phi, n); \
  theta = mul(theta, n); \
  REAL sin_theta = sin(theta); \
  w.v[0] = mul(sin_theta, cos(phi)); \
  w.v[1] = mul(sin_theta, sin(phi)); \
  w.v[2] = cos(theta); \
  w = add(mul(r2, w), c); \
}
MANDELBULB(Vec3Dual3f, Dual3f, float, float)
MANDELBULB(Vec3Dual3fx, Dual3fx, FloatX, floatx)
#undef MANDELBULB

// Triplex variant: power 8 only.
#define MANDELBULB(TRIPLEX, VEC) \
void MandelbulbTriplex(inout VEC w, in VEC c) \
{ \
  TRIPLEX z; \
  z.x = w.v[0]; \
  z.y = w.v[1]; \
  z.z = w.v[2]; \
  z = sqr(z); \
  z = sqr(z); \
  z = sqr(z); \
  z.z = neg(z.z); \
  w.v[0] = z.x; \
  w.v[1] = z.y; \
  w.v[2] = z.z; \
  w = add(w, c); \
}
MANDELBULB(TriplexDual3f, Vec3Dual3f)
MANDELBULB(TriplexDual3fx, Vec3Dual3fx)
#undef MANDELBULB
