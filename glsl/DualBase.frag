#donotrun
/*
Dual number template implementation file.
*/

struct DUAL
{
	MONO x;
	MONO d[DDIM];
};

DUAL dual(MONO x)
{
  DUAL d;
	d.x = x;
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mono(0);
	}
	return d;
}

DUAL dual(MONO x, int dim)
{
	DUAL d = dual(x);
	d.d[dim] = mono(1);
	return d;
}

DUAL dual(int x)
{
  DUAL d;
	d.x = mono(x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mono(0);
	}
	return d;
}

DUAL add(MONO x, DUAL y)
{
	DUAL d;
	d.x = add(x, y.x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = y.d[i];
	}
	return d;
}

DUAL add(DUAL x, MONO y)
{
	DUAL d;
	d.x = add(x.x, y);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = x.d[i];
	}
	return d;
}

DUAL add(DUAL x, DUAL y)
{
	DUAL d;
	d.x = add(x.x, y.x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = add(x.d[i], y.d[i]);
	}
	return d;
}

DUAL sub(MONO x, DUAL y)
{
	DUAL d;
	d.x = sub(x, y.x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = neg(y.d[i]);
	}
	return d;
}

DUAL sub(DUAL x, MONO y)
{
	DUAL d;
	d.x = sub(x.x, y);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = x.d[i];
	}
	return d;
}

DUAL sub(DUAL x, DUAL y)
{
	DUAL d;
	d.x = sub(x.x, y.x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = sub(x.d[i], y.d[i]);
	}
	return d;
}

DUAL neg(DUAL x)
{
	DUAL d;
	d.x = neg(x.x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = neg(x.d[i]);
	}
	return d;
}

DUAL mul(MONO x, DUAL y)
{
	DUAL d;
	d.x = mul(x, y.x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(x, y.d[i]);
	}
	return d;
}

DUAL mul(DUAL x, MONO y)
{
	DUAL d;
	d.x = mul(x.x, y);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(x.d[i], y);
	}
	return d;
}

DUAL mul(DUAL x, DUAL y)
{
	DUAL d;
	d.x = mul(x.x, y.x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = add(mul(x.x, y.d[i]), mul(x.d[i], y.x));
	}
	return d;
}

DUAL sqr(DUAL x)
{
	DUAL d;
	d.x = sqr(x.x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(mono(2), mul(x.x, x.d[i]));
	}
	return d;
}

DUAL div(MONO x, DUAL y)
{
	DUAL d;
	MONO s = inv(y.x);
	MONO ns2 = neg(sqr(s));
	d.x = mul(x, s);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(mul(x, y.d[i]), ns2);
	}
	return d;
}

DUAL div(DUAL x, MONO y)
{
	DUAL d;
	MONO s = inv(y);
	d.x = mul(x.x, s);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(x.d[i], s);
	}
	return d;
}

DUAL div(DUAL x, DUAL y)
{
	DUAL d;
	MONO s = inv(y.x);
	MONO s2 = sqr(s);
	d.x = mul(x.x, s);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(sub(mul(x.d[i], y.x), mul(x.x, y.d[i])), s2);
	}
	return d;
}

DUAL pow(DUAL x, MONO y)
{
	DUAL d;
	MONO s = mul(pow(x.x, sub(y, mono(1))), y);
	d.x = pow(x.x, y);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(x.d[i], s);
	}
	return d;
}

DUAL sqrt(DUAL x)
{
	DUAL d;
	d.x = sqrt(x.x);
	MONO s = div(mono(0.5), d.x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(x.d[i], s);
	}
	return d;
}

DUAL sin(DUAL x)
{
	DUAL d;
	d.x = sin(x.x);
	MONO s = cos(d.x);
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(x.d[i], s);
	}
	return d;
}

DUAL cos(DUAL x)
{
	DUAL d;
	d.x = cos(x.x);
	MONO s = neg(sin(d.x));
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(x.d[i], s);
	}
	return d;
}

DUAL tan(DUAL x)
{
	DUAL d;
	d.x = tan(x.x);
	MONO s = inv(sqr(cos(x.x)));
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(x.d[i], s);
	}
	return d;
}

DUAL atan(DUAL y, DUAL x)
{
	DUAL d;
	if (eq(x.x, mono(0)) && eq(y.x, mono(0)))
	{
		return dual(mono(0));
	}
	d.x = atan(y.x, x.x);
	MONO s = inv(add(sqr(x.x), sqr(y.x)));
	for (int i = 0; i < DDIM; ++i)
	{
		d.d[i] = mul(sub(mul(x.x, y.d[i]), mul(y.x, x.d[i])), s);
	}
	return d;
}

DUAL abs(DUAL x)
{
	if (lt(x.x, mono(0)))
	{
		return neg(x);
	}
	else
	{
		return x;
	}
}
