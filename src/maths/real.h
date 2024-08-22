#pragma once

#include <cmath>
#include <limits>
#include <utility>

#define USE_DOUBLE 1



// From PBRT
constexpr double DoubleOneMinusEpsilon = 0.99999999999999989;
constexpr float  FloatOneMinusEpsilon  = 0.99999994f;

#if USE_DOUBLE
using real = double;
constexpr real OneMinusEpsilon = DoubleOneMinusEpsilon;
#else
using real = float;
constexpr real OneMinusEpsilon = FloatOneMinusEpsilon;
#endif


constexpr real real_inf = std::numeric_limits<real>::infinity();
constexpr real pi       = static_cast<real>(3.141592653589793238462643383279);
constexpr real pi_half  = static_cast<real>(1.5707963267948966192313216916398);
constexpr real two_pi   = static_cast<real>(6.283185307179586476925286766559);


inline  float sqr( float a) { return a * a; }
inline double sqr(double a) { return a * a; }

// not in C++ stdlib, but should optimize to C extlib sincos()
inline std::pair< float,  float> sincos( float arg) { return { std::sin(arg), std::cos(arg) }; }
inline std::pair<double, double> sincos(double arg) { return { std::sin(arg), std::cos(arg) }; }
