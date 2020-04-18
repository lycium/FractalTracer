#donotrun
/*
Triplex template instantiation.
*/

#define TRIPLEX Triplexf
#define REAL float
#define real float
#include "TriplexBase.frag"
#undef real
#undef REAL
#undef TRIPLEX

#define TRIPLEX Triplexd
#define REAL double
#define real double
#include "TriplexBase.frag"
#undef real
#undef REAL
#undef TRIPLEX

#define TRIPLEX Triplexfx
#define REAL FloatX
#define real floatx
#include "TriplexBase.frag"
#undef real
#undef REAL
#undef TRIPLEX

#define TRIPLEX Triplexdx
#define REAL DoubleX
#define real doublex
#include "TriplexBase.frag"
#undef real
#undef REAL
#undef TRIPLEX

#define TRIPLEX TriplexDual3f
#define REAL Dual3f
#define real float
#include "TriplexBase.frag"
#undef real
#undef REAL
#undef TRIPLEX

#define TRIPLEX TriplexDual3d
#define REAL Dual3d
#define real double
#include "TriplexBase.frag"
#undef real
#undef REAL
#undef TRIPLEX

#define TRIPLEX TriplexDual3fx
#define REAL Dual3fx
#define real floatx
#include "TriplexBase.frag"
#undef real
#undef REAL
#undef TRIPLEX

#define TRIPLEX TriplexDual3dx
#define REAL Dual3dx
#define real doublex
#include "TriplexBase.frag"
#undef real
#undef REAL
#undef TRIPLEX
