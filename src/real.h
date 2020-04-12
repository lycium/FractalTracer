#pragma once

#include <limits>

#include "vec2.h"
#include "vec3.h"
#include "Dual.h"

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

using vec2r  = vec2<real>;
using vec3r  = vec3<real>;
using Dual3r = Dual<real, 3>;


constexpr static real real_inf = std::numeric_limits<real>::infinity();
