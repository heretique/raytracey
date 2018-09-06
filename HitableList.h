#pragma once

#include "hitable.h"
#include <vector>

class HitableList : public Hitable
{
public:
    bool hit(const math::Rayf& r, float tMin, float tMax, HitData& hitData) const override
    {
        HitData tmpHitData;
        bool    hitAnything  = false;
        double  closestSofar = tMax;
        for (const auto* hitable : list)
        {
            if (hitable->hit(r, tMin, closestSofar, tmpHitData))
            {
                hitAnything  = true;
                closestSofar = tmpHitData.t;
                hitData      = tmpHitData;
            }
        }

        return hitAnything;
    }

    std::vector<Hitable*> list;
};
