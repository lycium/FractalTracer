#donotrun
/*
Lambert W function template instantiation.
*/

#define REAL float
#define real float
#define REAL_EPS 1.1920928955078125e-7
#define gsl_sf_result glsl_sf_result_float
#include "LambertWBase.frag"
#undef gsl_sf_result
#undef REAL_EPS
#undef real
#undef REAL

#define REAL double
#define real double
#define REAL_EPS 2.2204460492503131e-16LF
#define gsl_sf_result glsl_sf_result_double
#include "LambertWBase.frag"
#undef gsl_sf_result
#undef REAL_EPS
#undef real
#undef REAL

#define REAL FloatX
#define real floatx
#define REAL_EPS floatx(1.1920928955078125e-7)
#define gsl_sf_result glsl_sf_result_floatx
#include "LambertWBase.frag"
#undef gsl_sf_result
#undef REAL_EPS
#undef real
#undef REAL

#define REAL DoubleX
#define real doublex
#define REAL_EPS doublex(2.2204460492503131e-16LF)
#define gsl_sf_result glsl_sf_result_doublex
#include "LambertWBase.frag"
#undef gsl_sf_result
#undef REAL_EPS
#undef real
#undef REAL
