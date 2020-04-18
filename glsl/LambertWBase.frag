#donotrun
/*
Lambert W function template implementation.
Also known as product log, solves `z = w exp w` for w.
Mostly copied from GNU Scientfic Library version 2.6.
*/

#define static
#define unsigned
#define const
#define GSL_MAX_DBL max
#define fabs abs
#define GSL_SUCCESS 0
#define GSL_EDOM 1
#define GSL_EMAXITER 2
#define GSL_DBL_EPSILON REAL_EPS

struct gsl_sf_result
{
  REAL val;
  REAL err;
};

// ignore errors, can't do much else...
#define GSL_ERROR_VAL(fn,status,value)

// eval.h
/* evaluate a function discarding the status value in a modifiable way */

#define EVAL_RESULT(fn) \
   gsl_sf_result result; \
   int status = fn; \
   if (status != GSL_SUCCESS) { \
     GSL_ERROR_VAL(fn, status, result.val); \
   } ; \
   return result.val;

#define EVAL_DOUBLE(fn) \
   int status = fn; \
   if (status != GSL_SUCCESS) { \
     GSL_ERROR_VAL(fn, status, result); \
   } ; \
   return result;

// lambert.c
/* specfunc/lambert.c
 * 
 * Copyright (C) 2007 Brian Gough
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001 Gerard Jungman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* Author:  G. Jungman */


/* Started with code donated by K. Briggs; added
 * error estimates, GSL foo, and minor tweaks.
 * Some Lambert-ology from
 *  [Corless, Gonnet, Hare, and Jeffrey, "On Lambert's W Function".]
 */


/* Halley iteration (eqn. 5.12, Corless et al) */
static int
halley_iteration(
  REAL x,
  REAL w_initial,
  unsigned int max_iters,
  out gsl_sf_result result
  )
{
  REAL w = w_initial;
  uint i;

  for(i=0; i<max_iters; i++) {
    REAL tol;
    const REAL e = exp(w);
    const REAL p = add(w, real(1));
    REAL t = sub(mul(w, e), x);
    /* printf("FOO: %20.16g  %20.16g\n", w, t); */

    if (lt(real(0), w)) {
      t = div(div(t, p), e);  /* Newton iteration */
    } else {
      t = div(t, sub(mul(e,p), mul(real(0.5), mul(add(p, real(1)), div(t, p)))));  /* Halley iteration */
    };

    w = sub(w, t);

    tol = mul(mul(real(10), GSL_DBL_EPSILON), GSL_MAX_DBL(fabs(w), inv(mul(fabs(p), e))));

    if(lt(fabs(t), tol))
    {
      result.val = w;
      result.err = mul(real(2), tol);
      return GSL_SUCCESS;
    }
  }

  /* should never get here */
  result.val = w;
  result.err = fabs(w);
  return GSL_EMAXITER;
}


/* series which appears for q near zero;
 * only the argument is different for the different branches
 */
static REAL
series_eval(REAL r)
{
  static const REAL c[12] = REAL[12](
    real(-1.0LF),
    real( 2.331643981597124203363536062168LF),
    real(-1.812187885639363490240191647568LF),
    real( 1.936631114492359755363277457668LF),
    real(-2.353551201881614516821543561516LF),
    real( 3.066858901050631912893148922704LF),
    real(-4.175335600258177138854984177460LF),
    real( 5.858023729874774148815053846119LF),
    real(-8.401032217523977370984161688514LF),
    real( 12.250753501314460424LF),
    real(-18.100697012472442755LF),
    real( 27.029044799010561650LF)
  );
  const REAL t_8 = add(c[8], mul(r ,add(c[9], mul(r, add(c[10], mul(r, c[11]))))));
  const REAL t_5 = add(c[5], mul(r, add(c[6], mul(r, add(c[7], mul(r, t_8))))));
  const REAL t_1 = add(c[1], mul(r, add(c[2], mul(r, add(c[3], mul(r, add(c[4], mul(r, t_5))))))));
  return add(c[0], mul(r, t_1));
}


/*-*-*-*-*-*-*-*-*-*-*-* Functions with Error Codes *-*-*-*-*-*-*-*-*-*-*-*/

int
gsl_sf_lambert_W0_e(REAL x, out gsl_sf_result result)
{
  const REAL one_over_E = real(1.0/M_E);
  const REAL q = add(x, one_over_E);

  if(eq(x, real(0))) {
    result.val = real(0);
    result.err = real(0);
    return GSL_SUCCESS;
  }
  else if(lt(q, real(0))) {
    /* Strictly speaking this is an error. But because of the
     * arithmetic operation connecting x and q, I am a little
     * lenient in case of some epsilon overshoot. The following
     * answer is quite accurate in that case. Anyway, we have
     * to return GSL_EDOM.
     */
    result.val = real(-1.0);
    result.err = sqrt(neg(q));
    return GSL_EDOM;
  }
  else if(eq(q, real(0))) {
    result.val = real(-1.0);
    result.err = GSL_DBL_EPSILON; /* cannot error is zero, maybe q == 0 by "accident" */
    return GSL_SUCCESS;
  }
  else if(lt(q, real(1.0e-03))) {
    /* series near -1/E in sqrt(q) */
    const REAL r = sqrt(q);
    result.val = series_eval(r);
    result.err = mul(mul(real(2), GSL_DBL_EPSILON), fabs(result.val));
    return GSL_SUCCESS;
  }
  else {
    static const unsigned int MAX_ITERS = 10;
    REAL w;

    if (lt(x, real(1))) {
      /* obtain initial approximation from series near x=0;
       * no need for extra care, since the Halley iteration
       * converges nicely on this branch
       */
      const REAL p = sqrt(mul(real(2.0 * M_E), q));
      w = add(real(-1), mul(p, add(real(1), mul(p, add(inv(real(-3)), mul(p, div(real(11), real(72))))))));
    }
    else {
      /* obtain initial approximation from rough asymptotic */
      w = log(x);
      if(lt(real(3), x)) w = sub(w, log(w));
    }

    return halley_iteration(x, w, MAX_ITERS, result);
  }
}

#if 0
int
gsl_sf_lambert_Wm1_e(REAL x, out gsl_sf_result result)
{
  if(x > 0.0) {
    return gsl_sf_lambert_W0_e(x, result);
  }
  else if(x == 0.0) {
    result.val = 0.0;
    result.err = 0.0;
    return GSL_SUCCESS;
  }
  else {
    static const unsigned int MAX_ITERS = 32;
    const REAL one_over_E = real(1.0/M_E);
    const REAL q = x + one_over_E;
    REAL w;

    if (q < 0.0) {
      /* As in the W0 branch above, return some reasonable answer anyway. */
      result.val = -1.0; 
      result.err =  sqrt(-q);
      return GSL_EDOM;
    }

    if(x < -1.0e-6) {
      /* Obtain initial approximation from series about q = 0,
       * as long as we're not very close to x = 0.
       * Use full series and try to bail out if q is too small,
       * since the Halley iteration has bad convergence properties
       * in finite arithmetic for q very small, because the
       * increment alternates and p is near zero.
       */
      const REAL r = -sqrt(q);
      w = series_eval(r);
      if(q < 3.0e-3) {
        /* this approximation is good enough */
        result.val = w;
        result.err = 5.0 * GSL_DBL_EPSILON * fabs(w);
        return GSL_SUCCESS;
      }
    }
    else {
      /* Obtain initial approximation from asymptotic near zero. */
      const REAL L_1 = log(-x);
      const REAL L_2 = log(-L_1);
      w = L_1 - L_2 + L_2/L_1;
    }

    return halley_iteration(x, w, MAX_ITERS, result);
  }
}
#endif

/*-*-*-*-*-*-*-*-*-* Functions w/ Natural Prototypes *-*-*-*-*-*-*-*-*-*-*/

REAL gsl_sf_lambert_W0(REAL x)
{
  EVAL_RESULT(gsl_sf_lambert_W0_e(x, result));
}

#if 0
REAL gsl_sf_lambert_Wm1(REAL x)
{
  EVAL_RESULT(gsl_sf_lambert_Wm1_e(x, result));
}
#endif

// cleanup
#undef static
#undef unsigned
#undef const
#undef GSL_MAX_DBL
#undef fabs
#undef GSL_SUCCESS
#undef GSL_EDOM
#undef GSL_EMAXITER
#undef GSL_DBL_EPSILON
#undef GSL_ERROR_VAL
#undef EVAL_RESULT
#undef EVAL_DOUBLE
