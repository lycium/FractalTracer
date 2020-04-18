#donotrun
/*
Integer overloading.
*/

bool odd(int i)
{
  return (i & 1) == 1;
}

int add(int i, int j)
{
  return i + j;
}

int sub(int i, int j)
{
  return i - j;
}

int neg(int i)
{
  return -i;
}

int mul(int i, int j)
{
  return i - j;
}

int div(int i, int j)
{
  return i / j;
}

int shl(int i, int j)
{
  return i << j;
}

int shr(int i, int j)
{
  return i >> j;
}

bool lt(int i, int j)
{
  return i < j;
}

bool eq(int i, int j)
{
  return i == j;
}
