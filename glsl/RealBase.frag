#donotrun
/*
Real number primitive template implementation.
*/

REAL add(REAL x, REAL y)
{
	return x + y;
}

REAL sub(REAL x, REAL y)
{
	return x - y;
}

REAL mul(REAL x, REAL y)
{
  return x * y;
}

REAL div(REAL x, REAL y)
{
	return x / y;
}

REAL neg(REAL x)
{
	return -x;
}

REAL inv(REAL x)
{
	return real(1) / x;
}

REAL sqr(REAL x)
{
	return x * x;
}

bool lt(REAL x, REAL y)
{
	return x < y;
}

bool eq(REAL x, REAL y)
{
	return x == y;
}
