#pragma once

#include "ray.h"

struct HitData
{
    float          t;
    math::Vector3f p;
    math::Vector3f normal;
};

class Hitable
{
public:
    virtual ~Hitable()
    {
    }

    virtual bool hit(const math::Rayf& r, float tMin, float tMax, HitData& hitData) const = 0;
};
