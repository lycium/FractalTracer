#donotrun
/*
Overload builtins.  See `Builtin.frag` for details.
*/

int abs(int x) { return _builtin_abs(x); }
ivec2 abs(ivec2 x) { return _builtin_abs(x); }
ivec3 abs(ivec3 x) { return _builtin_abs(x); }
ivec4 abs(ivec4 x) { return _builtin_abs(x); }
float abs(float x) { return _builtin_abs(x); }
vec2 abs(vec2 x) { return _builtin_abs(x); }
vec3 abs(vec3 x) { return _builtin_abs(x); }
vec4 abs(vec4 x) { return _builtin_abs(x); }
double abs(double x) { return _builtin_abs(x); }
dvec2 abs(dvec2 x) { return _builtin_abs(x); }
dvec3 abs(dvec3 x) { return _builtin_abs(x); }
dvec4 abs(dvec4 x) { return _builtin_abs(x); }

float sqrt(float x) { return _builtin_sqrt(x); }
vec2 sqrt(vec2 x) { return _builtin_sqrt(x); }
vec3 sqrt(vec3 x) { return _builtin_sqrt(x); }
vec4 sqrt(vec4 x) { return _builtin_sqrt(x); }

float inversesqrt(float x) { return _builtin_inversesqrt(x); }
vec2 inversesqrt(vec2 x) { return _builtin_inversesqrt(x); }
vec3 inversesqrt(vec3 x) { return _builtin_inversesqrt(x); }
vec4 inversesqrt(vec4 x) { return _builtin_inversesqrt(x); }

float length(float x) { return _builtin_length(x); }
float length(vec2 x) { return _builtin_length(x); }
float length(vec3 x) { return _builtin_length(x); }
float length(vec4 x) { return _builtin_length(x); }

float distance(float x, float y) { return _builtin_distance(x, y); }
float distance(vec2 x, vec2 y) { return _builtin_distance(x, y); }
float distance(vec3 x, vec3 y) { return _builtin_distance(x, y); }
float distance(vec4 x, vec4 y) { return _builtin_distance(x, y); }

float normalize(float x) { return _builtin_normalize(x); }
vec2 normalize(vec2 x) { return _builtin_normalize(x); }
vec3 normalize(vec3 x) { return _builtin_normalize(x); }
vec4 normalize(vec4 x) { return _builtin_normalize(x); }

float dot(float y, float x) { return _builtin_dot(y, x); }
float dot(vec2 y, vec2 x) { return _builtin_dot(y, x); }
float dot(vec3 y, vec3 x) { return _builtin_dot(y, x); }
float dot(vec4 y, vec4 x) { return _builtin_dot(y, x); }
double dot(double y, double x) { return _builtin_dot(y, x); }
double dot(dvec2 y, dvec2 x) { return _builtin_dot(y, x); }
double dot(dvec3 y, dvec3 x) { return _builtin_dot(y, x); }
double dot(dvec4 y, dvec4 x) { return _builtin_dot(y, x); }

vec3 cross(vec3 x, vec3 y) { return _builtin_cross(x, y); }
dvec3 cross(dvec3 x, dvec3 y) { return _builtin_cross(x, y); }

bool isnan(float x) { return _builtin_isnan(x); }
bvec2 isnan(vec2 x) { return _builtin_isnan(x); }
bvec3 isnan(vec3 x) { return _builtin_isnan(x); }
bvec4 isnan(vec4 x) { return _builtin_isnan(x); }
bool isnan(double x) { return _builtin_isnan(x); }
bvec2 isnan(dvec2 x) { return _builtin_isnan(x); }
bvec3 isnan(dvec3 x) { return _builtin_isnan(x); }
bvec4 isnan(dvec4 x) { return _builtin_isnan(x); }

bool isinf(float x) { return _builtin_isinf(x); }
bvec2 isinf(vec2 x) { return _builtin_isinf(x); }
bvec3 isinf(vec3 x) { return _builtin_isinf(x); }
bvec4 isinf(vec4 x) { return _builtin_isinf(x); }
bool isinf(double x) { return _builtin_isinf(x); }
bvec2 isinf(dvec2 x) { return _builtin_isinf(x); }
bvec3 isinf(dvec3 x) { return _builtin_isinf(x); }
bvec4 isinf(dvec4 x) { return _builtin_isinf(x); }

float sign(float x) { return _builtin_sign(x); }
vec2 sign(vec2 x) { return _builtin_sign(x); }
vec3 sign(vec3 x) { return _builtin_sign(x); }
vec4 sign(vec4 x) { return _builtin_sign(x); }
double sign(double x) { return _builtin_sign(x); }
dvec2 sign(dvec2 x) { return _builtin_sign(x); }
dvec3 sign(dvec3 x) { return _builtin_sign(x); }
dvec4 sign(dvec4 x) { return _builtin_sign(x); }

float modf(float x, out float i) { return _builtin_modf(x, i); }
vec2 modf(vec2 x, out vec2 i) { return _builtin_modf(x, i); }
vec3 modf(vec3 x, out vec3 i) { return _builtin_modf(x, i); }
vec4 modf(vec4 x, out vec4 i) { return _builtin_modf(x, i); }
double modf(double x, out double i) { return _builtin_modf(x, i); }
dvec2 modf(dvec2 x, out dvec2 i) { return _builtin_modf(x, i); }
dvec3 modf(dvec3 x, out dvec3 i) { return _builtin_modf(x, i); }
dvec4 modf(dvec4 x, out dvec4 i) { return _builtin_modf(x, i); }

float exp(float x) { return _builtin_exp(x); }
vec2 exp(vec2 x) { return _builtin_exp(x); }
vec3 exp(vec3 x) { return _builtin_exp(x); }
vec4 exp(vec4 x) { return _builtin_exp(x); }

float exp2(float x) { return _builtin_exp2(x); }
vec2 exp2(vec2 x) { return _builtin_exp2(x); }
vec3 exp2(vec3 x) { return _builtin_exp2(x); }
vec4 exp2(vec4 x) { return _builtin_exp2(x); }

float log(float x) { return _builtin_log(x); }
vec2 log(vec2 x) { return _builtin_log(x); }
vec3 log(vec3 x) { return _builtin_log(x); }
vec4 log(vec4 x) { return _builtin_log(x); }

float log2(float x) { return _builtin_log2(x); }
vec2 log2(vec2 x) { return _builtin_log2(x); }
vec3 log2(vec3 x) { return _builtin_log2(x); }
vec4 log2(vec4 x) { return _builtin_log2(x); }

float pow(float x, float y) { return _builtin_pow(x, y); }
vec2 pow(vec2 x, vec2 y) { return _builtin_pow(x, y); }
vec3 pow(vec3 x, vec3 y) { return _builtin_pow(x, y); }
vec4 pow(vec4 x, vec4 y) { return _builtin_pow(x, y); }

float sin(float x) { return _builtin_sin(x); }
vec2 sin(vec2 x) { return _builtin_sin(x); }
vec3 sin(vec3 x) { return _builtin_sin(x); }
vec4 sin(vec4 x) { return _builtin_sin(x); }

float cos(float x) { return _builtin_cos(x); }
vec2 cos(vec2 x) { return _builtin_cos(x); }
vec3 cos(vec3 x) { return _builtin_cos(x); }
vec4 cos(vec4 x) { return _builtin_cos(x); }

float tan(float x) { return _builtin_tan(x); }
vec2 tan(vec2 x) { return _builtin_tan(x); }
vec3 tan(vec3 x) { return _builtin_tan(x); }
vec4 tan(vec4 x) { return _builtin_tan(x); }

float asin(float x) { return _builtin_asin(x); }
vec2 asin(vec2 x) { return _builtin_asin(x); }
vec3 asin(vec3 x) { return _builtin_asin(x); }
vec4 asin(vec4 x) { return _builtin_asin(x); }

float acos(float x) { return _builtin_acos(x); }
vec2 acos(vec2 x) { return _builtin_acos(x); }
vec3 acos(vec3 x) { return _builtin_acos(x); }
vec4 acos(vec4 x) { return _builtin_acos(x); }

float atan(float x) { return _builtin_atan(x); }
vec2 atan(vec2 x) { return _builtin_atan(x); }
vec3 atan(vec3 x) { return _builtin_atan(x); }
vec4 atan(vec4 x) { return _builtin_atan(x); }

float sinh(float x) { return _builtin_sinh(x); }
vec2 sinh(vec2 x) { return _builtin_sinh(x); }
vec3 sinh(vec3 x) { return _builtin_sinh(x); }
vec4 sinh(vec4 x) { return _builtin_sinh(x); }

float cosh(float x) { return _builtin_cosh(x); }
vec2 cosh(vec2 x) { return _builtin_cosh(x); }
vec3 cosh(vec3 x) { return _builtin_cosh(x); }
vec4 cosh(vec4 x) { return _builtin_cosh(x); }

float tanh(float x) { return _builtin_tanh(x); }
vec2 tanh(vec2 x) { return _builtin_tanh(x); }
vec3 tanh(vec3 x) { return _builtin_tanh(x); }
vec4 tanh(vec4 x) { return _builtin_tanh(x); }

float asinh(float x) { return _builtin_asinh(x); }
vec2 asinh(vec2 x) { return _builtin_asinh(x); }
vec3 asinh(vec3 x) { return _builtin_asinh(x); }
vec4 asinh(vec4 x) { return _builtin_asinh(x); }

float acosh(float x) { return _builtin_acosh(x); }
vec2 acosh(vec2 x) { return _builtin_acosh(x); }
vec3 acosh(vec3 x) { return _builtin_acosh(x); }
vec4 acosh(vec4 x) { return _builtin_acosh(x); }

float atanh(float x) { return _builtin_atanh(x); }
vec2 atanh(vec2 x) { return _builtin_atanh(x); }
vec3 atanh(vec3 x) { return _builtin_atanh(x); }
vec4 atanh(vec4 x) { return _builtin_atanh(x); }

float atan(float y, float x) { return _builtin_atan(y, x); }
vec2 atan(vec2 y, vec2 x) { return _builtin_atan(y, x); }
vec3 atan(vec3 y, vec3 x) { return _builtin_atan(y, x); }
vec4 atan(vec4 y, vec4 x) { return _builtin_atan(y, x); }

int max(int y, int x) { return _builtin_max(y, x); }
ivec2 max(ivec2 y, ivec2 x) { return _builtin_max(y, x); }
ivec3 max(ivec3 y, ivec3 x) { return _builtin_max(y, x); }
ivec4 max(ivec4 y, ivec4 x) { return _builtin_max(y, x); }
float max(float y, float x) { return _builtin_max(y, x); }
vec2 max(vec2 y, vec2 x) { return _builtin_max(y, x); }
vec3 max(vec3 y, vec3 x) { return _builtin_max(y, x); }
vec4 max(vec4 y, vec4 x) { return _builtin_max(y, x); }
double max(double y, double x) { return _builtin_max(y, x); }
dvec2 max(dvec2 y, dvec2 x) { return _builtin_max(y, x); }
dvec3 max(dvec3 y, dvec3 x) { return _builtin_max(y, x); }
dvec4 max(dvec4 y, dvec4 x) { return _builtin_max(y, x); }
