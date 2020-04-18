#donotrun
/*
Implement double precision versions of various functions.
Contents mostly taken from FragM distribution with some fixes.
The builtin capture and overloading is split into separate files.
*/

// broadcasting

#define V(f) \
dvec2 f(dvec2 x) { return dvec2(f(x.x), f(x.y)); } \
dvec3 f(dvec3 x) { return dvec3(f(x.x), f(x.y), f(x.z)); } \
dvec4 f(dvec4 x) { return dvec4(f(x.x), f(x.y), f(x.z), f(x.w)); }

#define V2(f) \
dvec2 f(dvec2 x, dvec2 y) { return dvec2(f(x.x, y.x), f(x.y, y.y)); } \
dvec3 f(dvec3 x, dvec3 y) { return dvec3(f(x.x, y.x), f(x.y, y.y), f(x.z, y.z)); } \
dvec4 f(dvec4 x, dvec4 y) { return dvec4(f(x.x, y.x), f(x.y, y.y), f(x.z, y.z), f(x.w, y.w)); }

// constants

const double M_PI = 3.14159265358979323846LF;
const double M_2PI = M_PI*2.0LF;
const double M_PI2 = M_PI/2.0LF;
const double M_E = 2.71828182845904523536LF;
const double M_EHALF = 1.6487212707001281469LF;

// implement improved square root
// built in square root, length, distance, normalize is not accurate

// Hardware sqrt improved by the Babylonian algorithm (Newton Raphson)
double sqrt(double m)
{
  if (! (m > 0.0LF)) { return 0.0LF; } // FIXME return NaN for < 0?
  double z = _builtin_sqrt(m);
  z = (z + m / z) / 2.0LF;
  z = (z + m / z) / 2.0LF;
  z = (z + m / z) / 2.0LF;
  z = (z + m / z) / 2.0LF;
  return z;
}
V(sqrt)

// implement improved length

double length(double x) { return abs(x); }
double length(dvec2 x) { return sqrt(dot(x, x)); } // FIXME overflow / underflow
double length(dvec3 x) { return sqrt(dot(x, x)); } // FIXME overflow / underflow
double length(dvec4 x) { return sqrt(dot(x, x)); } // FIXME overflow / underflow

// implement improved distance

double distance(double x, double y) { return length(x - y); }
double distance(dvec2 x, dvec2 y) { return length(x - y); }
double distance(dvec3 x, dvec3 y) { return length(x - y); }
double distance(dvec4 x, dvec4 y) { return length(x - y); }

// implement improved normalize

double normalize(double x) { double l = length(x); return l > 0.0LF ? x / l : 0.0LF; }
dvec2 normalize(dvec2 x) { double l = length(x); return l > 0.0LF ? x / l : dvec2(0.0LF); }
dvec3 normalize(dvec3 x) { double l = length(x); return l > 0.0LF ? x / l : dvec3(0.0LF); }
dvec4 normalize(dvec4 x) { double l = length(x); return l > 0.0LF ? x / l : dvec4(0.0LF); }

//--------------------------------------------------------------------
// 11/12/17
// double cos() sin() remez exp() by FractalForums.org user clacker
//--------------------------------------------------------------------
// 11/13/17
// remez double atan() by FractalForums.org user 3dickulus
// remez double log() gist.github.com/dhermes modified for FragM

const int TrigIter = 5;
const double TrigLimit = 1;

// sine

double sin( double x ){
  int i;
  int counter = 0;
  double sum = x, t = x;
  double s = x;

  if(isnan(x) || isinf(x))
    return 0.0LF;

  while(abs(s) > TrigLimit){
    s = s/3.0;
    counter += 1;
  }

  sum = s;
  t = s;

  for(i=1;i<=TrigIter;i++)
  {
    t=(t*(-1.0)*s*s)/(2.0*double(i)*(2.0*double(i)+1.0));
    double old_sum = sum;
    sum=sum+t;
    if (old_sum == sum) break;
  }

  for(i=0;i<counter;i++)
    sum = 3.0*sum - 4.0*sum*sum*sum;

  return sum;
}
V(sin)

// cosine

double cos( double x ){
  int i;
  int counter = 0;
  double sum = 1, t = 1;
  double s = x;

  if(isnan(x) || isinf(x))
    return 0.0LF;

  while(abs(s) > TrigLimit){
    s = s/3.0;
    counter += 1;
  }

  for(i=1;i<=TrigIter;i++)
  {
        t=t*(-1.0)*s*s/(2.0*double(i)*(2.0*double(i)-1.0));
        double old_sum = sum;
        sum=sum+t;
        if (old_sum == sum) break;
  }

  for(i=0;i<counter;i++)
    sum = -3.0*sum + 4.0*sum*sum*sum;

  return sum;
}
V(cos)

/* Approximation of f(x) = exp(x)
 * on interval [ 0, 1.0 ]
 * with a polynomial of degree 10.
 */
double exp_approx( double x ) {
    double u = 4.5714785424007307e-7LF;
    u = u * x + 2.2861717525121477e-6LF;
    u = u * x + 2.5459354562599535e-5LF;
    u = u * x + 1.9784992840356075e-4LF;
    u = u * x + 1.389195677460962e-3LF;
    u = u * x + 8.3332264219304372e-3LF;
    u = u * x + 4.1666689828374069e-2LF;
    u = u * x + 1.6666666374456794e-1LF;
    u = u * x + 5.0000000018885691e-1LF;
    u = u * x + 9.9999999999524239e-1LF;
    u = u * x + 1.0000000000000198LF;
  if(isnan(u) || isinf(u))
    return 0.0LF;
    return u;
}

double exp(double x){

  int i;
  int n;
   double f;
   double e_accum = M_E;
   double answer = 1.0LF;
  bool invert_answer = false;

  // if x is negative, convert to positive and take inverse at end
   if(x < 0.0){
    x = -x;
    invert_answer = true;
  }

  // break i into integer andfractional parts
   n = int(x);
   f = x - double(n);

  // put f in the range 0-0.5 and adjust answer
  // subtract 0.5 from fractional exponent and
  // add 0.5 to integer exponent by multiplying answer by exp(0.5)
  if(f > 0.5){
    f -= 0.5;
    answer = M_EHALF;
  }

   for(i=0;i<8;i++){
    if(((n >> i) & 1) == 1)
      answer *= e_accum;
    e_accum *= e_accum;
  }

  answer *= exp_approx(f);

   if(invert_answer)
    answer = 1.0/answer;

  return answer;
}
V(exp)

double tan(double x) {
    return sin(x)/cos(x);
}
V(tan)

/* Approximation of f(x) = log(x)
 * on interval [ 0.5, 1.0 ]
 * with a polynomial of degree 7. */
double log_approx(double x)
{
    double u = 1.3419648079694775LF;
    u = u * x + -8.159823646011416LF;
    u = u * x + 2.1694837976736115e+1LF;
    u = u * x + -3.3104943376189169e+1LF;
    u = u * x + 3.2059105806949116e+1LF;
    u = u * x + -2.0778140811001331e+1LF;
    u = u * x + 9.8897820531599449LF;
    return u * x + -2.9427826194103015LF;
}

// ln_ieee754(double x)

// https://gist.github.com/dhermes/105da2a3c9861c90ea39
// Accuracy: the error is always less than 1 ulp
// modified for FragM by 3Dickulus @ FractalForums.org
double log(double x)  {

  x += 4.94065645841247E-308LF;

  double
    Ln2Hi = 6.93147180369123816490e-01LF, /* 3fe62e42 fee00000 */
    Ln2Lo = 1.90821492927058770002e-10LF, /* 3dea39ef 35793c76 */
    L0    = 7.0710678118654752440e-01LF,  /* 1/sqrt(2) */
    L1    = 6.666666666666735130e-01LF,   /* 3FE55555 55555593 */
    L2    = 3.999999999940941908e-01LF,   /* 3FD99999 9997FA04 */
    L3    = 2.857142874366239149e-01LF,   /* 3FD24924 94229359 */
    L4    = 2.222219843214978396e-01LF,   /* 3FCC71C5 1D8E78AF */
    L5    = 1.818357216161805012e-01LF,   /* 3FC74664 96CB03DE */
    L6    = 1.531383769920937332e-01LF,   /* 3FC39A09 D078C69F */
    L7    = 1.479819860511658591e-01LF;   /* 3FC2F112 DF3E5244 */

  // special cases
  if( isinf(x) )
        return double(1.0/0.0); /* return +inf */
  if( isnan(x) || x < 0 )
        return double(-0.0); /* nan */
  if( x == 0 )
        return double(-1.0/0.0); /* return -inf */

    // Argument Reduction
    int ki;
    double f1 = frexp(x, ki);

    if (f1 < L0) {
    f1 += f1;
    ki--;
  }

  double f = f1 - 1.0;
  double k = double(ki);

  // Approximation
  double s = f / (2.0 + f);
  double s2 = s * s;
  double s4 = s2 * s2;
    // Terms with odd powers of s^2.
  double t1 = s2 * (L1 + s4 * (L3 + s4 * (L5 + s4 * L7)));
    // Terms with even powers of s^2.
  double t2 = s4 * (L2 + s4 * (L4 + s4 * L6));
  double R = t1 + t2;
  double hfsq = 0.5 * f * f;

    return k*Ln2Hi - ((hfsq - (s*(hfsq+R) + k*Ln2Lo)) - f);

}
V(log)

double log2(double N)
{
    return (log(N) / 0.69314718055995LF);
}
V(log2)

double pow(double a, double b) {

return exp(log(a) * b);

}
V2(pow)

double exp2(double a)
{
  return exp(log(2.0LF) * a);
}
V(exp2)

/* Approximation of f(x) = atan(x)
 * on interval [ 0.0 , π/4 ]
 * with a polynomial of degree 10.
 */
double atan_approx(double x)
{
    double u = -2.9140257478972053e-3LF;
    u = u * x + 3.2005571699830107e-2LF;
    u = u * x + -1.627659300903442e-1LF;
    u = u * x + 5.0513223367120972e-1LF;
    u = u * x + -1.0595254685451083LF;
    u = u * x + 1.5689337521140527LF;
    u = u * x + -1.6640521237136246LF;
    u = u * x + 1.270853367426007LF;
    u = u * x + -7.356602708332424e-1LF;
    u = u * x + 4.0096549787833572e-1LF;
    u = u * x + -2.317084220916499e-1LF;
    u = u * x + 5.5673464120677798e-2LF;
    u = u * x + 6.1518997985636844e-2LF;
    u = u * x + 3.4871637890152628e-3LF;
    u = u * x + -9.1551371689992248e-2LF;
    u = u * x + 9.5405115942529782e-5LF;
    u = u * x + 1.1109982274527962e-1LF;
    u = u * x + 1.0462503881004859e-6LF;
    u = u * x + -1.4285721713962809e-1LF;
    u = u * x + 3.9206483284047854e-9LF;
    u = u * x + 1.9999999985236683e-1LF;
    u = u * x + 3.7405487051591751e-12LF;
    u = u * x + -3.3333333333339171e-1LF;
    u = u * x + 4.8455084038412012e-16LF;
    u = u * x + 1.0LF;
    u = u * x + 8.8145999826527008e-22LF;

    if(isnan(u) || isinf(u))
        return double(0.0);
    return u;
}

double atan(double y, double x){
    double ay = abs(y), ax = abs(x);
    bool inv = (ay > ax);
    double z;
    if(inv) z = ax/ay; else z = ay/ax; // [0,1]
    double th = atan_approx(z);        // [0,π/4]
    if(inv) th = M_PI2 - th;           // [0,π/2]
    if(x < 0.0) th = M_PI - th;        // [0,π]
    if(y < 0.0) th = -th;              // [-π,π]
    return th;
}
V2(atan)

double atan(double yx)
{
  return atan(yx, double(1));
}
V(atan)

// double versions of sinh, cosh, tanh

double sinh(double val) {
  double tmp = exp(val); // FIXME expm1 may be better near 0
  return (tmp - 1.0LF / tmp) / 2.0LF;
}
V(sinh)

double cosh(double val) {
  double tmp = exp(val);
  return (tmp + 1.0LF / tmp) / 2.0LF;
}
V(cosh)

double tanh(double val) {
  double tmp = exp(val); // FIXME expm1 may be better near 0
  return (tmp - 1.0LF / tmp) / (tmp + 1.0LF / tmp);
}
V(tanh)

// TODO: asin acos asinh acosh atanh

#undef V
#undef V2
