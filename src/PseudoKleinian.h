#pragma once

#include "DualDEObject.h"



// Pseudo-Kleinian DE by Knighty
struct DualPseudoKleinianIteration final : public IterationFunction
{
    real mins[4] = { -0.8323f, -0.694f, -0.5045f, 0.8067f };
    real maxs[4] = {  0.8579f,  1.0883f, 0.8937f, 0.9411f };

    virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
    {
        Dual3r px = p_in.x;
        Dual3r py = p_in.y;
        Dual3r pz = p_in.z;

        px = clamp(px, mins[0], maxs[0]) * 2 - px;
        py = clamp(py, mins[1], maxs[1]) * 2 - py;
        pz = clamp(pz, mins[2], maxs[2]) * 2 - pz;

        const real k = std::max(mins[3] / length2(DualVec3r{ px, py, pz }), (real)1);
        p_out = DualVec3r(px, py, pz) * k;
    }

    virtual real getPower() const noexcept override final { return 1; }

    virtual IterationFunction * clone() const override
    {
        return new DualPseudoKleinianIteration(*this);
    }
};
