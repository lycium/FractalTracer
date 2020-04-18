#donotrun
/*
Distance estimates for z -> a*z, z -> z^p, z -> a*z^p.
Client should implement `formula()` which does one iteration step.
At the first iteration `z = c`.
*/

void formula(inout Vec3Dual3f z, in Vec3Dual3f c);
void formula(inout Vec3Dual3fx z, in Vec3Dual3fx c);

#group DistanceEstimate
// Maximum number of times to iterate the formula
uniform int Iterations; slider[0,100,10000]
// Stop iterating when bigger than this
uniform float Log2EscapeRadius; slider[0,10,10000]
// Force using AValue for the `a` scaling, instead of autodetect
uniform bool OverrideA; checkbox[false]
// The `a` value in `z -> a z^p`
uniform float AValue; slider[0.001,1,1000]
// Force using PValue for the `p` scaling, instead of autodetect
uniform bool OverrideP; checkbox[false]
// The `p` value in `z -> a z^p`
uniform float PValue; slider[0.001,1,1000]
// Force rounding the `p` scaling to an integer
uniform bool RoundPToInteger; checkbox[false]
// Scale calculated DE (may reduce overstepping)
uniform float DEScale; slider[0.0001,0.25,10]

FloatX EscapeRadius = exp2(floatx(Log2EscapeRadius));
FloatX EscapeRadius2 = sqr(EscapeRadius);

float DistanceEstimate(vec3 pos, out vec3 normal)
{
  // iterate
  int n;
  Vec3Dual3f c;
  c.v[0] = dual3f(pos.x, 0);
  c.v[1] = dual3f(pos.y, 1);
  c.v[2] = dual3f(pos.z, 2);
  Vec3Dual3f z = c;
  for (n = 0; n < Iterations; ++n)
  {
    if (! lt(dot(z, z).x, float(2)))
    {
      break;
    }
    formula(z, c);
  }
  // promote to FloatX
  // because after initial escape, values explode quickly
  Vec3Dual3fx cc;
  for (int i = 0; i < 3; ++i)
  {
    cc.v[i].x = floatx(c.v[i].x);
    for (int j = 0; j < 3; ++j)
    {
      cc.v[i].d[j] = floatx(c.v[i].d[j]);
    }
  }
  Vec3Dual3fx zz;
  for (int i = 0; i < 3; ++i)
  {
    zz.v[i].x = floatx(z.v[i].x);
    for (int j = 0; j < 3; ++j)
    {
      zz.v[i].d[j] = floatx(z.v[i].d[j]);
    }
  }
  // iterate some more
  for (; n < Iterations; ++n)
  {
    if (! lt(dot(zz, zz).x, EscapeRadius2))
    {
      break;
    }
    formula(zz, cc);
  }
  // get last 3 iterates
  FloatX z0 = length(zz).x;
  FloatX Z = z0;
  // compute derivative and normal
  FloatX u[3];
  u[0] = zz.v[0].x;
  u[1] = zz.v[1].x;
  u[2] = zz.v[2].x;
  {
    // normalize u
    FloatX s = floatx(0);
    for (int i = 0; i < 3; ++i)
    {
      s = add(s, sqr(u[i]));
    }
    s = inv(sqrt(s));
    for (int i = 0; i < 3; ++i)
    {
      u[i] = mul(s, u[i]);
    }
  }
  FloatX J[3][3];
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      J[i][j] = zz.v[j].d[i];
    }
  }
  FloatX v[3];
  FloatX dZ;
  {
    FloatX s = floatx(0);
    for (int i = 0; i < 3; ++i)
    {
      v[i] = floatx(0);
      for (int j = 0; j < 3; ++j)
      {
        v[i] = add(v[i], mul(J[i][j], u[j]));
      }
      s = add(s, sqr(v[i]));
    }
    s = sqrt(s);
    dZ = s;
    s = inv(s);
    for (int i = 0; i < 3; ++i)
    {
      normal[i] = convert_float(mul(s, v[i]));
    }
  }
  // iterate 2 more times for estimates of scaling
  formula(zz, cc);
  FloatX z1 = length(zz).x;
  formula(zz, cc);
  FloatX z2 = length(zz).x;
  // logs
  float log_z0 = convert_float(log(z0));
  float log_z1 = convert_float(log(z1));
  float log_z2 = convert_float(log(z2));
  float log_Z = log_z0;
  float log_R = convert_float(log(EscapeRadius));
  // estimate power
  float p = (log_z2 - log_z1) / (log_z1 - log_z0);
  if (OverrideP)
  {
    p = PValue;
  }
  if (RoundPToInteger)
  {
    p = round(p);
  }
  float log_p = log(p);
  // estimate scaling
  float log_a = ((log_z2 - p * log_z1) + (log_z1 - p * log_z0)) * 0.5;
  if (OverrideA)
  {
    log_a = log(AValue);
  }
  // compute final results
  float /*f, */df;
  if (log_a == 0) // a == 1
  {
    // f = log(log_Z / log_R) / log_p;
    df = convert_float(div(dZ, mul(Z, mul(log_Z, log_p))));
  }
  else if (p == 1)
  {
    // f = (log_Z - log_R) / log_a;
    df = convert_float(div(dZ, mul(Z, log_a)));
  }
  else
  {
    FloatX w = gsl_sf_lambert_W0
      (mul(pow(floatx(p), floatx(log_Z / log_a)), floatx(log_p * log_R / log_a)));
    // f = log_Z / log_a - w / log_p;
    df = convert_float(div(dZ, mul(Z, mul(add(w, floatx(1)), floatx(log_a)))));
  }
  float de = float(1) / df;
  if (isnan(de) || isinf(de))
  {
    return 0.0;
  }
  return DEScale * de;
}
