#pragma once

#include "hitable.h"
#include "utils.h"
#include <vector>

class HitableList : public Hitable
{
public:
    bool hit(const math::Rayf& r, float tMin, float tMax, HitData& hitData) const override
    {
        HitData tmpHitData;
        bool    hitAnything  = false;
        float   closestSofar = tMax;
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

    bool boundingBox(float tMin, float tMax, math::AABBf& bbox) const override
    {
        using namespace math;

        if (list.size() < 1)
            return false;

        AABBf tmpBbox;
        bool  result = list[0]->boundingBox(tMin, tMax, tmpBbox);
        if (!result)
            return false;

        for (size_t i = 0; i < list.size(); ++i)
        {
            if (list[i]->boundingBox(tMin, tMax, tmpBbox))
            {
                bbox = surroundingBbox(bbox, tmpBbox);
            }
            else
                return false;
        }

        return true;
    }

    std::vector<Hitable*> list;
};
