#pragma once

#include "ray.h"

class Material;

struct HitData
{
    float          t;
    math::Vector3f p;
    math::Vector3f normal;
    Material*      materialPtr;
};

class Hitable
{
public:
    virtual ~Hitable()
    {
    }

    virtual bool hit(const math::Rayf& r, float tMin, float tMax, HitData& hitData) const = 0;
};
