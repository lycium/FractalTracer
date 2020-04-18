#donotrun
/*
Vector template implementation.
*/

struct VEC
{
  SCALAR v[VDIM];
};

VEC vec(SCALAR x)
{
  VEC v;
  for (int i = 0; i < VDIM; ++i)
  {
    v.v[i] = x;
  }
  return v;
}

VEC vec(SCALAR[VDIM] x)
{
  VEC v;
  for (int i = 0; i < VDIM; ++i)
  {
    v.v[i] = x[i];
  }
  return v;
}

VEC add(VEC x, VEC y)
{
  VEC v;
  for (int i = 0; i < VDIM; ++i)
  {
    v.v[i] = add(x.v[i], y.v[i]);
  }
  return v;
}

VEC sub(VEC x, VEC y)
{
  VEC v;
  for (int i = 0; i < VDIM; ++i)
  {
    v.v[i] = sub(x.v[i], y.v[i]);
  }
  return v;
}

VEC neg(VEC x)
{
  VEC v;
  for (int i = 0; i < VDIM; ++i)
  {
    v.v[i] = neg(x.v[i]);
  }
  return v;
}

VEC abs(VEC x)
{
  VEC v;
  for (int i = 0; i < VDIM; ++i)
  {
    v.v[i] = abs(x.v[i]);
  }
  return v;
}

VEC mul(VEC x, SCALAR y)
{
  VEC v;
  for (int i = 0; i < VDIM; ++i)
  {
    v.v[i] = mul(x.v[i], y);
  }
  return v;
}

VEC mul(SCALAR x, VEC y)
{
  VEC v;
  for (int i = 0; i < VDIM; ++i)
  {
    v.v[i] = mul(x, y.v[i]);
  }
  return v;
}

VEC mul(VEC x, VEC y)
{
  VEC v;
  for (int i = 0; i < VDIM; ++i)
  {
    v.v[i] = mul(x.v[i], y.v[i]);
  }
  return v;
}

VEC div(VEC x, SCALAR y)
{
  VEC v;
  for (int i = 0; i < VDIM; ++i)
  {
    v.v[i] = div(x.v[i], y);
  }
  return v;
}

SCALAR dot(VEC x, VEC y)
{
  SCALAR s = scalar(0);
  for (int i = 0; i < VDIM; ++i)
  {
    s = add(s, mul(x.v[i], y.v[i]));
  }
  return s;
}

SCALAR length(VEC x)
{
  return sqrt(dot(x, x));
}

VEC normalize(VEC x)
{
  return div(x, length(x));
}

#if VDIM == 3
VEC cross(VEC a, VEC b)
{
  VEC v;
  v.v[0] = sub(mul(a.v[1], b.v[2]), mul(a.v[2], b.v[1]));
  v.v[1] = sub(mul(a.v[2], b.v[0]), mul(a.v[0], b.v[2]));
  v.v[2] = sub(mul(a.v[0], b.v[1]), mul(a.v[1], b.v[0]));
  return v;
}
#endif
