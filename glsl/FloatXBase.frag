#donotrun
/*
Extended exponent float template implementation file.
*/

struct FLOATX
{
  MANTISSA m;
  EXPONENT e;
};

FLOATX floatx(float m, EXPONENT e)
{
  FLOATX f;
  f.m = mantissa(frexp(m, f.e));
  f.e = add(f.e, e);
  if (eq(f.m, mantissa(0)))
  {
    f.e = EXPONENT_MIN;
  }
  return f;
}

FLOATX floatx(double m, EXPONENT e)
{
  FLOATX f;
  f.m = mantissa(frexp(m, f.e));
  f.e = add(f.e, e);
  if (eq(f.m, mantissa(0)))
  {
    f.e = EXPONENT_MIN;
  }
  return f;
}

FLOATX floatx(float m)
{
  return floatx(m, 0);
}

FLOATX floatx(double m)
{
  return floatx(m, 0);
}

float convert_float(FLOATX f)
{
  return ldexp(float(f.m), f.e);
}

double convert_double(FLOATX f)
{
  return ldexp(double(f.m), f.e);
}

FLOATX abs(FLOATX a)
{
  FLOATX f;
  f.m = abs(a.m);
  f.e = a.e;
  return f;
}

FLOATX mul(FLOATX a, FLOATX b)
{
  return floatx(mul(a.m, b.m), add(a.e, b.e));
}

FLOATX mul(FLOATX a, MANTISSA b)
{
  return floatx(mul(a.m, b), a.e);
}

FLOATX mul(MANTISSA a, FLOATX b)
{
  return floatx(mul(a, b.m), b.e);
}

FLOATX div(FLOATX a, FLOATX b)
{
  return floatx(div(a.m, b.m), sub(a.e, b.e));
}

FLOATX add(FLOATX a, FLOATX b)
{
  if (lt(b.e, a.e))
  {
    return floatx(add(a.m, ldexp(b.m, sub(b.e, a.e))), a.e);
  }
  else
  {
    return floatx(add(ldexp(a.m, sub(a.e, b.e)), b.m), b.e);
  }
}

FLOATX neg(FLOATX a)
{
  FLOATX f;
  f.m = neg(a.m);
  f.e = a.e;
  return f;
}

FLOATX inv(FLOATX a)
{
  return floatx(inv(a.m), neg(a.e));
}

FLOATX sub(FLOATX a, FLOATX b)
{
  return add(a, neg(b));
}

FLOATX sqr(FLOATX a)
{
  return mul(a, a);
}

FLOATX sqrt(FLOATX a)
{
  return floatx
    ( sqrt(odd(a.e) ? mul(mantissa(2), a.m) : a.m)
    , odd(a.e) ? div(sub(a.e, exponent(1)), exponent(2)) : div(a.e, exponent(2))
    );
}

FLOATX log(FLOATX a)
{
  return floatx(add(log(a.m), mul(log(mantissa(2)), mantissa(a.e))));
}

FLOATX log2(FLOATX a)
{
  return floatx(add(log2(a.m), mantissa(a.e)));
}

FLOATX pow(FLOATX x, EXPONENT n)
{
  FLOATX y = floatx(mantissa(1));
  if (eq(n, exponent(0))) return y;
  while (lt(exponent(1), n))
  {
    if (odd(n))
      y = mul(y, x);
    x = sqr(x);
    n = shr(n, 1);
	}
  return mul(x, y);
}

FLOATX exp(FLOATX a) // FIXME exponent constants depend on types
{
  if (lt(a.e, EXPONENT(-53))) return floatx(mantissa(1));
  if (lt(a.e, EXPONENT(7))) return floatx(exp(ldexp(a.m, a.e)));
  EXPONENT E_MIN = exponent(floor(log(float(EXPONENT_MAX))));
  if (lt(E_MIN, a.e)) return floatx(mantissa(lt(a.m, 0) ? 0.0 : 1.0/0.0));
  return pow(floatx(exp(a.m)), shl(EXPONENT(1), a.e));
}

FLOATX pow(FLOATX a, FLOATX b)
{
  return exp(mul(log(a), b));
}

FLOATX exp2(FLOATX a)
{
  return pow(floatx(2), a);
}

FLOATX sin(FLOATX a)
{
  if (lt(a.e, exponent(-100))) return a; // FIXME depends on types
  return floatx(sin(ldexp(a.m, a.e)));
}

FLOATX cos(FLOATX a)
{
  return floatx(cos(ldexp(a.m, a.e)));
}

FLOATX tan(FLOATX a)
{
  if (lt(a.e, exponent(-100))) return a; // FIXME depends on types
  return floatx(tan(ldexp(a.m, a.e)));
}

FLOATX atan(FLOATX y, FLOATX x)
{
  EXPONENT e = neg(max(y.e, x.e));
  return floatx(atan(ldexp(y.e, e), ldexp(x.m, e)));
}

bool lt(FLOATX a, FLOATX b)
{
  if (lt(mantissa(0), a.m))
  {
    if (lt(b.m, mantissa(0))) return false;
    if (lt(b.e, a.e)) return false;
    if (lt(a.e, b.e)) return true;
    return lt(a.m, b.m);
  }
  else
  {
    if (lt(mantissa(0), b.m)) return true;
    if (lt(b.e, a.e)) return true;
    if (lt(a.e, b.e)) return false;
    return lt(a.m, b.m);
  }
}

bool eq(FLOATX a, FLOATX b)
{
  return eq(a.e, b.e) && eq(a.m, b.m);
}

FLOATX max(FLOATX a, FLOATX b)
{
  return lt(a, b) ? b : a;
}
