#pragma once

#include "hitable.h"
#include "utils.h"
#include <memory>

class Sphere : public Hitable
{
public:
    Sphere() {}
    ~Sphere() override {}
    Sphere(math::Vector3f center, float radius, std::unique_ptr<Material> material,
           math::Vector3f velocity = math::Vector3f(0.f, 0.f, 0.f))
        : center(center)
        , radius(radius)
        , material(std::move(material))
        , velocity(velocity)
    {
    }
    bool hit(const math::Rayf& r, float tMin, float tMax, HitData& hitData) const override
    {
        using namespace math;
        Vector3f oc           = r.origin() - getCenter(r.time());
        float    a            = dot(r.direction(), r.direction());
        float    b            = dot(oc, r.direction());
        float    c            = dot(oc, oc) - radius * radius;
        float    discriminant = b * b - a * c;
        if (discriminant > 0.f)
        {
            hitData.materialPtr = material.get();
            float temp          = (-b - sqrt(b * b - a * c)) / a;
            if (temp < tMax && temp > tMin)
            {
                hitData.t      = temp;
                hitData.p      = r.pointOnRay(temp);
                hitData.normal = (hitData.p - getCenter(r.time())) / radius;
                return true;
            }
            temp = (-b + sqrt(b * b - a * c)) / a;
            if (temp < tMax && temp > tMin)
            {
                hitData.t      = temp;
                hitData.p      = r.pointOnRay(temp);
                hitData.normal = (hitData.p - getCenter(r.time())) / radius;
                return true;
            }
        }

        return false;
    }

    bool boundingBox(float tMin, float tMax, math::AABBf& bbox) const override
    {
        using namespace math;
        if (cmpf(tMin, tMax, util::epsilon))
        {
            bbox = sphereBbox(getCenter(tMin), radius);
        }
        else
        {
            AABBf bbox1 = sphereBbox(getCenter(tMin), radius);
            AABBf bbox2 = sphereBbox(getCenter(tMax), radius);
            bbox        = surroundingBbox(bbox1, bbox2);
        }

        return true;
    }

    math::Vector3f getCenter(const float time) const
    {
        return center + velocity * time;
    }

public:
    math::Vector3f            center;
    float                     radius;
    std::unique_ptr<Material> material;
    math::Vector3f            velocity;
};
