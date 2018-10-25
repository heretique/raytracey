#pragma once

#include <Hq/Math/AABB.h>
#include <Hq/Math/Ray.h>

class Material;

struct HitData
{
    float              t;
    hq::math::Vector3f p;
    hq::math::Vector3f normal;
    Material*          materialPtr;
};

class Hitable
{
public:
    virtual ~Hitable() {}

    virtual bool hit(const hq::math::Rayf& r, float tMin, float tMax, HitData& hitData) const = 0;
    virtual bool boundingBox(float tMin, float tMax, hq::math::AABBf& bbox) const             = 0;
};
