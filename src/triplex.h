#pragma once

#include "vec.h"

// lifted vector operations

template <typename real_type>
struct triplex
{
  vec<3, real_type> v;

  inline triplex(const triplex &t) : v(t.v) { }
  inline triplex(const vec<3, real_type> v) : v(v) { }
  inline triplex(const real_type &x, const real_type &y, const real_type &z) : v(x, y, z) { }
  inline triplex(const real_type &x, const real_type &y) : v(x, y, real_type(0)) { }
  inline triplex(const real_type &x) : v(x, real_type(0), real_type(0)) { }
  inline triplex() { } // uninitialized is a good thing? not sure..

  operator const vec<3, real_type> & () const { return v; }

  triplex & operator=(const triplex &t) = default;

  const real_type & x() const { return v.x(); }
  const real_type & y() const { return v.y(); }
  const real_type & z() const { return v.z(); }
};

template <typename real_type>
triplex<real_type> operator+(const triplex<real_type> &a, const triplex<real_type> &b)
{
  return a.v + b.v;
}

template <typename real_type>
triplex<real_type> operator-(const triplex<real_type> &a, const triplex<real_type> &b)
{
  return a.v - b.v;
}

template <typename real_type>
triplex<real_type> operator-(const triplex<real_type> &b)
{
  return -b.v;
}

template <typename real_type>
triplex<real_type> operator*(const triplex<real_type> &a, const real_type &b)
{
  return a.v * b;
}

template <typename real_type>
triplex<real_type> operator*(const real_type &a, const triplex<real_type> &b)
{
  return a * b.v;
}

template <typename real_type>
triplex<real_type> operator/(const triplex<real_type> &a, const real_type &b)
{
  return a.v * (1/b);
}

template <typename real_type>
triplex<real_type> operator/(const real_type &a, const triplex<real_type> &b)
{
  return (1/a) * b.v;
}

template <typename real_type>
triplex<real_type> length2(const triplex<real_type> &a)
{
  return length2(a.v);
}

// custom triplex operations

template <typename real_type>
triplex<real_type> operator*(const triplex<real_type> &a, const triplex<real_type> &b)
{
  real_type arho(sqrt(sqr(a.x()) + sqr(a.y())));
  real_type brho(sqrt(sqr(b.x()) + sqr(b.y())));
  real_type A(real_type(1) - a.z() * b.z() / (arho * brho));
  return
    { A * (a.x() * b.x() - a.y() * b.y())
    , A * (a.x() * b.y() + a.y() * b.x())
    , arho * b.z() + brho * a.z()
    };
}

template <typename real_type>
triplex<real_type> operator/(const triplex<real_type> &a, const triplex<real_type> &b)
{
  real_type arho(sqrt(sqr(a.x()) + sqr(a.y())));
  real_type brho(sqrt(sqr(b.x()) + sqr(b.y())));
  real_type A(real_type(1) + a.z() * b.z() / (arho * brho));
  return triplex<real_type>
    ( A * (a.x() * b.x() + a.y() * b.y())
    , A * (a.y() * b.x() - a.x() * b.y())
    , brho * a.z() - arho * b.z()
    ) / length2(b);
}

template <typename real_type>
triplex<real_type> sqr(const triplex<real_type> &a)
{
  real_type x2(sqr(a.x()));
  real_type y2(sqr(a.y()));
  real_type arho2 = x2 + y2;
  real_type A = real_type(1) - sqr(a.z()) / arho2;
  return triplex<real_type>
    ( A * (x2 - y2)
    , A * real_type(2) * a.x() * a.y()
    , real_type(2) * sqrt(arho2) * a.z()
    );
}

using triplexi = triplex<int>;
using triplexr = triplex<real>;
using triplexf = triplex<float>;
using triplexd = triplex<double>;

using dualtriplex3r = triplex<Dual3r>;
using dualtriplex3f = triplex<Dual3f>;
using dualtriplex3d = triplex<Dual3d>;
