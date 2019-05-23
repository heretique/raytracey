#pragma once

#include "hitable.h"
#include <memory>
#include <Hq/Math/Utils.h>
#include <Hq/Utils.h>

void GetSphereUV(const hq::math::Vector3f& p, float& u, float& v)
{
    float phi   = atan2(p.z, p.x);
    float theta = asin(p.y);
    u           = 1.0f - (phi + M_PI) / (2 * M_PI);
    v           = (theta + M_PI / 2) / M_PI;
}

class Sphere : public Hitable
{
public:
    Sphere() {}
    ~Sphere() override {}
    Sphere(hq::math::Vector3f center, float radius, std::unique_ptr<Material> material,
           hq::math::Vector3f velocity = hq::math::Vector3f(0.f, 0.f, 0.f))
        : center(center)
        , radius(radius)
        , material(std::move(material))
        , velocity(velocity)
    {
    }
    bool hit(const hq::math::Rayf& r, float tMin, float tMax, HitData& hitData) const override
    {
        using namespace hq::math;
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
                GetSphereUV(hitData.normal, hitData.uv.u, hitData.uv.v);
                return true;
            }
            temp = (-b + sqrt(b * b - a * c)) / a;
            if (temp < tMax && temp > tMin)
            {
                hitData.t      = temp;
                hitData.p      = r.pointOnRay(temp);
                hitData.normal = (hitData.p - getCenter(r.time())) / radius;
                GetSphereUV(hitData.normal, hitData.uv.u, hitData.uv.v);
                return true;
            }
        }

        return false;
    }

    bool boundingBox(float tMin, float tMax, hq::math::AABBf& bbox) const override
    {
        using namespace hq::math;
        if (hq::cmpf(tMin, tMax, hq::util::epsilon))
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

    hq::math::Vector3f getCenter(const float time) const
    {
        return center + velocity * time;
    }

public:
    hq::math::Vector3f        center;
    float                     radius;
    std::unique_ptr<Material> material;
    hq::math::Vector3f        velocity;
};
