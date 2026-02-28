#pragma once

#include "scene_objects/DualDEObject.h"



// Pseudo-Kleinian DE by Knighty
struct DualPseudoKleinianIteration final : public IterationFunction
{
    real mins[4] = { -0.8323f, -0.694f, -0.5045f, 0.8067f };
    real maxs[4] = {  0.8579f,  1.0883f, 0.8937f, 0.9411f };

    virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
    {
        const Dual3r px = clamp(p_in.x(), mins[0], maxs[0]) * 2 - p_in.x();
        const Dual3r py = clamp(p_in.y(), mins[1], maxs[1]) * 2 - p_in.y();
        const Dual3r pz = clamp(p_in.z(), mins[2], maxs[2]) * 2 - p_in.z();

        const real k = std::max(mins[3] / length2(DualVec3r{ px, py, pz }), (real)1);
        p_out = DualVec3r(px, py, pz) * k;
    }

    virtual real getPower() const noexcept override final { return 1; }

    virtual IterationFunction * clone() const override final
    {
        return new DualPseudoKleinianIteration(*this);
    }

    virtual std::vector<ParamInfo> getParams() override
    {
        return {
            { "Mins", ParamInfo::Real4, mins, -2.0f, 2.0f },
            { "Maxs", ParamInfo::Real4, maxs, -2.0f, 2.0f },
        };
    }
};
