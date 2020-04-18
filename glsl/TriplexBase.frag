#donotrun
/*
Triplex template implementation.
Ref: <http://www.bugman123.com/Hypercomplex/index.html#MandelbulbZ>
*/

struct TRIPLEX
{
  REAL x, y, z;
};

TRIPLEX mul(TRIPLEX a, TRIPLEX b)
{
  REAL arho = sqrt(add(sqr(a.x), sqr(a.y)));
  REAL brho = sqrt(add(sqr(b.x), sqr(b.y)));
  REAL A = sub(real(1), div(mul(a.z, b.z), mul(arho, brho)));
  TRIPLEX r;
  r.x = mul(A, sub(mul(a.x, b.x), mul(a.y, b.y)));
  r.y = mul(A, add(mul(b.x, a.y), mul(a.x, b.y)));
  r.z = add(mul(arho, b.z), mul(brho, a.z));
  return r;
}

TRIPLEX sqr(TRIPLEX a)
{
  REAL x2 = sqr(a.x), y2 = sqr(a.y);
  REAL arho2 = add(x2, y2);
  REAL A = sub(real(1), div(sqr(a.z), arho2));
  TRIPLEX r;
  r.x = mul(A, sub(x2, y2));
  r.y = mul(A, mul(real(2), mul(a.x, a.y)));
  r.z = mul(mul(real(2), sqrt(arho2)), a.z);
  return r;
}
