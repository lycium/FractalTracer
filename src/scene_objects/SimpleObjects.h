#pragma once

#include "SceneObject.h"



struct Sphere final : public SceneObject
{
	vec3r centre = { 0, 0, 0 };
	real  radius = 1; 


	virtual real intersect(const Ray & r) noexcept override
	{
		const vec3r s = r.o - centre;
		const real  b = dot(s, r.d);
		const real  c = dot(s, s) - radius * radius;

		const real discriminant = b * b - c;
		if (discriminant < 0)
			return -1;

		// Compute both roots and return the nearest one that's > 0
		const real t1 = -b - std::sqrt(discriminant);
		const real t2 = -b + std::sqrt(discriminant);
		return (t1 >= 0) ? t1 : t2;
	}

	virtual vec3r getNormal(const vec3r & p) noexcept override
	{
		return (p - centre) * (1 / radius);
	}

	virtual SceneObject * clone() const override final
	{
		Sphere * o = new Sphere;
		*o = *this;
		return o;
	}
};


struct Quad final : public SceneObject
{
	const vec3r p, u, v, v0, v1, n;
	const real  d, inv_area;


	Quad(const vec3r & p_, const vec3r & u_, const vec3r & v_) :
		 p(p_), u(u_), v(v_),
		v0(u * (1 / dot(u, u))),
		v1(v * (1 / dot(v, v))),
		 n(       cross(u, v)  * (1 / length(cross(u, v)))),
		 d(dot(p, cross(u, v)) * (1 / length(cross(u, v)))),
		                 inv_area(1 / length(cross(u, v)))
		{ }

	virtual real intersect(const Ray & r) noexcept override
	{
		const real     den =      dot(n, r.d); if (std::fabs(den) <= ray_epsilon) return -1; // parallel to plane
		const real plane_t = (d - dot(n, r.o)) / den; if (plane_t <= ray_epsilon) return -1; // plane behind ray
		const vec3r s = r.o + r.d * plane_t - p;
		const real  u = dot(s, v0); if (u < 0 || u >= 1) return -1;
		const real  v = dot(s, v1); if (v < 0 || v >= 1) return -1;

		return plane_t;
	}

	virtual vec3r getNormal(const vec3r & p) noexcept override { return n; }

	virtual SceneObject * clone() const override final
	{
		Quad * o = new Quad(p, u, v);
		o->mat = mat;
		//*o = *this;
		return o;
	}
};
