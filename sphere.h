#pragma once

#include "hitable.h"

class Sphere : public Hitable
{
public:
    Sphere()
    {
    }
    Sphere(math::Vector3f center, float radius)
        : center(center)
        , radius(radius)
    {
    }
    bool hit(const math::Rayf& r, float tMin, float tMax, HitData& hitData) const override
    {
        using namespace math;
        Vector3f oc           = r.origin() - center;
        float    a            = dot(r.direction(), r.direction());
        float    b            = dot(oc, r.direction());
        float    c            = dot(oc, oc) - radius * radius;
        float    discriminant = b * b - a * c;
        if (discriminant > 0.f)
        {
            float temp = (-b - sqrt(b * b - a * c)) / a;
            if (temp < tMax && temp > tMin)
            {
                hitData.t      = temp;
                hitData.p      = r.pointOnRay(temp);
                hitData.normal = (hitData.p - center) / radius;
                return true;
            }
            temp = (-b + sqrt(b * b - a * c)) / a;
            if (temp < tMax && temp > tMin)
            {
                hitData.t      = temp;
                hitData.p      = r.pointOnRay(temp);
                hitData.normal = (hitData.p - center) / radius;
                return true;
            }
        }

        return false;
    }

    math::Vector3f center;
    float          radius;
};
