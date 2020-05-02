#pragma once

#include <limits>



#define USE_DOUBLE 0


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
constexpr real two_pi = static_cast<real>(6.283185307179586476925286766559);
