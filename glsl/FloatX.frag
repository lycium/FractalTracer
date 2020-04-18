#donotrun
/*
Extended exponent float template instantiation file.
*/

#define MANTISSA float
#define mantissa float
#define EXPONENT int
#define exponent int
#define EXPONENT_MAX ( 0x10000000)
#define EXPONENT_MIN (-0x10000000)
#define FLOATX FloatX
#define floatx floatx
#include "FloatXBase.frag"
#undef floatx
#undef FLOATX
#undef EXPONENT_MIN
#undef EXPONENT_MAX
#undef exponent
#undef EXPONENT
#undef mantissa
#undef MANTISSA

#define MANTISSA double
#define mantissa double
#define EXPONENT int
#define exponent int
#define EXPONENT_MAX ( 0x10000000)
#define EXPONENT_MIN (-0x10000000)
#define FLOATX DoubleX
#define floatx doublex
#include "FloatXBase.frag"
#undef floatx
#undef FLOATX
#undef EXPONENT_MIN
#undef EXPONENT_MAX
#undef exponent
#undef EXPONENT
#undef mantissa
#undef MANTISSA
