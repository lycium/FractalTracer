#pragma once

#include <algorithm>

#include "SceneObject.h"



// Base class for dual number based distance estimated (DE) objects
struct DualDEObject : public SceneObject
{
	vec3r centre = { 0, 0, 0 };
	real  radius = 1;
	real  bailout_radius2 = 65536;
	real  step_scale = 1; // Method of last resort to prevent overstepping, interpreted as a Lipschitz constant


	real getLinearDE(const DualVec3r & p_os, vec3r & normal_os_out) const noexcept
	{
		// Extract the position vector and Jacobian
		const vec3r p  = vec3r{ p_os.x.v[0], p_os.y.v[0], p_os.z.v[0] };
		const vec3r jx = vec3r{ p_os.x.v[1], p_os.y.v[1], p_os.z.v[1] };
		const vec3r jy = vec3r{ p_os.x.v[2], p_os.y.v[2], p_os.z.v[2] };
		const vec3r jz = vec3r{ p_os.x.v[3], p_os.y.v[3], p_os.z.v[3] };

		const real len2 = dot(p, p);
		const real len = sqrt(len2);
		const vec3r u = p * (1 / len); // Normalise p first to avoid overflow in dot products

		// Vector-matrix norm: ||J||_u = |u.J|/|u|
		// Ref: https://fractalforums.org/fractal-image-gallery/18/burning-ship-distance-estimation/647/msg3207#msg3207
		const vec3r dr = vec3r
		{
			dot(u, jx),
			dot(u, jy),
			dot(u, jz)
		};

		// The basic DE formula has no log for functions that escape like |z_{n+k}| ~ |z_n|*{a^k}
		const real len_dr = length(dr);

		const real de = (len - radius) / len_dr;

		if (std::isfinite(len_dr)) // TODO: this function is probably slow, find a replacement
		{
			// At some parts of the fractal, m can become NaN (hairs),
			// which pollutes everything downstream.
			// Calling code should deal with it.
			normal_os_out = normalise(dr);
			return de;
		}
		else
		{
			// The derivatives have overflowed to infinity
			// and then further operations on them yield NaN.
			// Assuming m is finite it might as well return 0 here.
			normal_os_out = vec3r{ 0,0,0 };
			return 0;
		}
	}

	real getPolynomialDE(const DualVec3r & p_os, vec3r & normal_os_out) const noexcept
	{
		// Extract the position vector and Jacobian
		const vec3r p  = vec3r{ p_os.x.v[0], p_os.y.v[0], p_os.z.v[0] };
		const vec3r jx = vec3r{ p_os.x.v[1], p_os.y.v[1], p_os.z.v[1] };
		const vec3r jy = vec3r{ p_os.x.v[2], p_os.y.v[2], p_os.z.v[2] };
		const vec3r jz = vec3r{ p_os.x.v[3], p_os.y.v[3], p_os.z.v[3] };

		const real len2 = dot(p, p);
		const real len = sqrt(len2);
		const vec3r u = p * (1 / len); // Normalise p first to avoid overflow in dot products

		// Vector-matrix norm: ||J||_u = |u.J|/|u|
		// Ref: https://fractalforums.org/fractal-image-gallery/18/burning-ship-distance-estimation/647/msg3207#msg3207
		const vec3r dr = vec3r
		{
			dot(u, jx),
			dot(u, jy),
			dot(u, jz)
		};

#if 0
		// Should work in theory, but unfortunately does not.
		// Ref: https://www.evl.uic.edu/hypercomplex/html/book/book.pdf chapter 9.6
		return 1.0f * len / length(dr);
#else
		// Modifications and explanations by claude to take into account polynomial-ness:
		const real len_dr = length(dr);

		// The basic DE formula has a log for functions that escape like |z_{n+k}| ~ |z_n|^{p^k}
		// Ref: http://linas.org/art-gallery/mderive/mderive.html
		const real de_base = len * std::log(len) / len_dr;

		// Koebe 1/4 theorem for complex-analytic functions implies that
		// the distance estimate is accurate up to a factor of 2.  Mandelbulb
		// is not complex-analytic (or even complex) but one might hope that
		// similar arguments could apply.  Divide by 2 to get the lower bound.
		const real koebe_factor = 0.5f;

#if 0
		// The distance estimate needs a log(power) factor.
		// Not sure if this is included in the derivative calculation.
		// Ref: https://fractalforums.org/fractal-mathematics-and-new-theories/28/extension-of-numerical-de-bounds-to-other-powersdimensions/3004
		const real power_factor = 0.48089834696298780245330822700063; // 1 / std::log(8.0);
#else
		const real power_factor = 1;
#endif

		const real de = koebe_factor * power_factor * de_base;

		if (std::isfinite(len_dr)) // TODO: this function is probably slow, find a replacement
		{
			// At some parts of the fractal, m can become NaN (hairs),
			// which pollutes everything downstream.
			// Calling code should deal with it.
			normal_os_out = normalise(dr);
			return de;
		}
		else
		{
			// The derivatives have overflowed to infinity
			// and then further operations on them yield NaN.
			// Assuming m is finite it might as well return 0 here.
			normal_os_out = vec3r{ 0,0,0 };
			return 0;
		}
#endif
	}

	// A DE for hybrids.
	// Ref: https://mathr.co.uk/de
	// a:                scale (not used?);
	// p:                power;
	// w:                current pos & Jacobian;
	// normal_os_output: normal vector to output
	real getHybridDEClaude(const real a, const real p, const DualVec3r & w, vec3r & normal_os_out) const noexcept
	{
		// Extract the position vector and Jacobian
		const vec3r v  = vec3r{ w.x.v[0], w.y.v[0], w.z.v[0] };
		const vec3r jx = vec3r{ w.x.v[1], w.y.v[1], w.z.v[1] };
		const vec3r jy = vec3r{ w.x.v[2], w.y.v[2], w.z.v[2] };
		const vec3r jz = vec3r{ w.x.v[3], w.y.v[3], w.z.v[3] };

		const real len2 = dot(v, v);
		const real len = std::sqrt(len2);
		const vec3r u = v * (1 / len); // Normalise p first to avoid overflow in dot products

		// Vector-matrix norm: ||J||_u = |u.J|/|u|
		// Ref: https://fractalforums.org/fractal-image-gallery/18/burning-ship-distance-estimation/647/msg3207#msg3207
		const vec3r dr = vec3r
		{
			dot(u, jx),
			dot(u, jy),
			dot(u, jz)
		};
		const real len_dr = length(dr);

		// The hybrid DE formula
		// Ref: https://mathr.co.uk/de
		const real k = len / len_dr;
		const real de_base =
			(p == 1) ? k * std::log(a) :
			(a == 1) ? k * std::log(p) * std::log(len) : k * (std::log(p) / (p - 1)) * (std::log(a) + (p - 1) * std::log(len));

		// Koebe 1/4 theorem for complex analytic functions says d is
		// valid up to a factor of 2 either way, we need the lower bound.
		// Mandelbulb etc are not complex-analytic, but hope for the best...
		//const real koebe_factor = 0.5f; // Knighty: should not be used IMHO. We already have step_scale.

		// Distance estimate needs a log(power) factor taken out.
		// Not sure of the justification, but it seems to work better this way,
		// and it makes it match the other DE modes.
#if 1
		const real power_factor = (p == 1) ? 1 : 1 / log(p);
#else
		const real power_factor = 1;
#endif
		const real de = power_factor * de_base; // * koebe_factor;

		if (std::isfinite(len_dr)) // TODO: this function is probably slow, find a replacement
		{
			// At some parts of the fractal, m can become NaN (hairs),
			// which pollutes everything downstream.
			// Calling code should deal with it.
			normal_os_out = normalise(dr);
			return de;
		}
		else
		{
			// The derivatives have overflowed to infinity
			// and then further operations on them yield NaN.
			// Assuming m is finite it might as well return 0 here.
			normal_os_out = 0;
			return 0;
		}
	}

	// Another DE for hybrids, by Knighty:
	// a:             Estimate of the bounding volume size of the whole fractal;
	// p:             Product of formulas' powers;
	// pmax:          Product of formulas' powers for all iterations;
	// w:             Current pos & Jacobian;
	// normal_os_out: Normal vector to output.
	real getHybridDEKnighty(const real p, const real max_pow, const DualVec3r & w, vec3r & normal_os_out) const noexcept
	{
		// Extract the position vector and Jacobian
		const vec3r v  = vec3r{ w.x.v[0], w.y.v[0], w.z.v[0] };
		const vec3r jx = vec3r{ w.x.v[1], w.y.v[1], w.z.v[1] };
		const vec3r jy = vec3r{ w.x.v[2], w.y.v[2], w.z.v[2] };
		const vec3r jz = vec3r{ w.x.v[3], w.y.v[3], w.z.v[3] };

		const real len2 = dot(v, v);
		const real len = sqrt(len2);
		const vec3r u = v * (1 / len); // Normalise p first to avoid overflow in dot products

		// Vector-matrix norm: ||J||_u = |u.J|/|u|
		// Ref: https://fractalforums.org/fractal-image-gallery/18/burning-ship-distance-estimation/647/msg3207#msg3207
		// ------
		// Knighty: It is possible to directly use a norm of the jacobian instead :)
		//  In practice, max(length(ji)) works well. Unfortunately there are some problems with functions like abs()--> discontinuity
		//  Another thing worth to try is to evaluate dr in the direction of the ray.
#if 1
		const vec3r dr = vec3r
		{
			dot(u, jx),
			dot(u, jy),
			dot(u, jz)
		};
		const real len_dr = length(dr);
#else
		const vec3r dr = vec3r
		{
			dot(u, jx),
			dot(u, jy),
			dot(u, jz)
		};
		const real len_dr = std::max(length(jx), std::max(length(jy), length(jz))); // std::sqrt(dot(jx,jx) + dot(jy,jy) + dot(jz,jz));
#endif

		// Strictly speaking the terms (1 - (bvr ^ (1 / max_pow) / r ^ (1 / p))) and (1 - p / max_pow * log(bvr) / log(r))
		// are not absolutely required because at the limit of high iteration counts they approach 1.
		// but they give more accurate results for low iteration count.
		// Notice that the formula is different from the one in the document. Here the formulas were tweaked for finite/low bail out radius.
		// Ok, it seems a little over complicated. Next, we can try to see if it can be simplified without getting visible artifacts.
		// Notice also that when the formulas have power == 1, we use only the second formula which reduces to : k * (1 - a / len) = (len - a) / len_dr
		const real k = len / len_dr;
		const real de = (p > 10000)
			// ff * r / dr * (log(r) - p / max_pow * log(bvr)) = ff * r / dr * log(r) * (1 - p / max_pow * log(bvr) / log(r));
			? k * (std::log(len) - p / max_pow * std::log(radius))
			// ff * r / dr * p * (1 - (bvr ^ (1 / max_pow) / r^(1 / p)));
			: k * p * (1 - std::pow(radius , 1 / max_pow) / std::pow(len , 1 / p));

		if (std::isfinite(len_dr)) // TODO: this function is probably slow, find a replacement
		{
			// At some parts of the fractal, m can become NaN (hairs),
			// which pollutes everything downstream.
			// Calling code should deal with it.
			normal_os_out = normalise(dr);
			return de;
		}
		else
		{
			// The derivatives have overflowed to infinity
			// and then further operations on them yield NaN.
			// Assuming m is finite it might as well return 0 here.
			normal_os_out = 0;
			return 0;
		}
	}

	// Get the distance estimate and normal vector for point p in object space
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) noexcept = 0;

	// Dual numbers provide exact normals as part of the evaluation
	virtual vec3r getNormal(const vec3r & p) noexcept override final
	{
		const DualVec3r p_dual(Dual3r(p.x, 0), Dual3r(p.y, 1), Dual3r(p.z, 2));

		vec3r normal_os;
		const real de_ignored = getDE(p_dual, normal_os);
		(void) de_ignored;
		return normal_os;
	}

	virtual real intersect(const Ray & r) noexcept override final
	{
		const vec3r s = r.o - centre;
		const real  b = dot(s, r.d);
		const real  c = dot(s, s) - radius * radius;

		const real discriminant = b * b - c;
		if (discriminant < 0)
			return -1;

		// Compute bounding interval
		const real t1 = -b - std::sqrt(discriminant);
		const real t2 = -b + std::sqrt(discriminant);
		if (t2 <= ray_epsilon) return -1;

		// Ray could be inside bounding sphere, start from ray epsilon
		const real thresh = DE_thresh;
		real t = std::max(ray_epsilon, t1);
		while (t < t2)
		{
			const vec3r p_os = s + r.d * t;
			const DualVec3r p_os_dual(Dual3r(p_os.x, 0), Dual3r(p_os.y, 1), Dual3r(p_os.z, 2));
	
			vec3r normal_ignored;
			const real DE = getDE(p_os_dual, normal_ignored) * step_scale;
			(void) normal_ignored;
			t += DE;

			// If we're close enough to the surface, return a valid intersection
			if (DE < thresh)
				return t;
		}

		return -1; // No intersection found
	}
};


struct IterationFunction
{
	virtual void init(const DualVec3r & p_0) noexcept { }
	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept = 0;
	virtual real getPower() const noexcept = 0;

	virtual IterationFunction * clone() const = 0;
};


struct GeneralDualDE final : public DualDEObject
{
	const std::vector<IterationFunction *> funcs;
	const std::vector<char> sequence;

	const int max_iters;
	const real max_pow;


	GeneralDualDE(
		const std::vector<IterationFunction *> funcs_,
		const std::vector<char> & sequence_,
		const int max_iters_) : funcs(funcs_), sequence(sequence_), max_iters(max_iters_), max_pow(getMaxPower()) { }

	// Copy constructor
	GeneralDualDE(const GeneralDualDE & v) :
		DualDEObject(v), funcs(cloneFuncs(v)), sequence(v.sequence), max_iters(v.max_iters), max_pow(v.max_pow) { }

	virtual ~GeneralDualDE()
	{
		for (IterationFunction * f : funcs)
			delete f;
	}

	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) noexcept override final
	{
		DualVec3r p = p_os;

		const int seq_len = (int)sequence.size();
		const int num_funcs = (int)funcs.size();
		for (int i = 0; i < num_funcs; ++i)
			funcs[i]->init(p);

		real current_pow = 1; // Needed for getHybridKnighty
		int seq_idx = 0;
		for (int i = 0; i < max_iters; i++)
		{
			DualVec3r p_new;
			funcs[sequence[seq_idx]]->eval(p, p_new);
			p = p_new;
			current_pow *= funcs[sequence[seq_idx]]->getPower();

			const real r2 = p.x.v[0] * p.x.v[0] + p.y.v[0] * p.y.v[0] + p.z.v[0] * p.z.v[0];
			if (r2 > bailout_radius2)
				break;

			seq_idx = nextSeqIdx(seq_idx);
		}
#if 1
		return getHybridDEKnighty(current_pow, max_pow, p, normal_os_out); // TODO: bounding volume! (1st argument)
#else
#if 1
		return getHybridDE(1, 8, p, normal_os_out);
#else
		return getPolynomialDE(w, normal_os_out);
#endif
#endif
	}

	virtual SceneObject * clone() const override
	{
		return new GeneralDualDE(*this);
	}

private:
	// Compute max_power and set bounding volume size of the fractal
	real getMaxPower() const
	{
		int seq_idx = 0;
		real max_p = 1;
		for (int i = 0; i < max_iters; i++)
		{
			max_p *= funcs[sequence[seq_idx]]->getPower();
			seq_idx = nextSeqIdx(seq_idx);
		}

		return max_p;
	}

	const std::vector<IterationFunction *> cloneFuncs(const GeneralDualDE & d) const
	{
		std::vector<IterationFunction *> f;
		f.resize(d.funcs.size());
		for (size_t i = 0; i < d.funcs.size(); ++i)
			f[i] = d.funcs[i]->clone();

		return f;
	}

	// Increment sequence idx with wraparound
	inline int nextSeqIdx(int i) const { return (i < (int)sequence.size() - 1) ? i + 1 : 0; }
};
