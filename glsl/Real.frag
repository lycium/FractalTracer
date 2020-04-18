#donotrun
/*
Real number primitive template instantiation.
*/

#define REAL float
#define real float
#include "RealBase.frag"
#undef real
#undef REAL

#define REAL double
#define real double
#include "RealBase.frag"
#undef real
#undef REAL
