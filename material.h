#pragma once

#include "hitable.h"
#include <Hq/Math/Utils.h>
#include <Hq/Rng.h>

class Material
{
public:
    virtual ~Material() {}
    virtual bool scatter(const hq::math::Rayf& rayIn, const HitData& hitData, hq::math::Vector3f& attenuation,
                         hq::math::Rayf& scattered) const = 0;
};

class Lambertian : public Material
{
public:
    Lambertian(const hq::math::Vector3f& albedo)
        : albedo(albedo)
    {
    }

    bool scatter(const hq::math::Rayf& rayIn, const HitData& hitData, hq::math::Vector3f& attenuation,
                 hq::math::Rayf& scattered) const override
    {
        using namespace hq::math;
        Vector3f target = hitData.p + hitData.normal + RandomInUnitSphere();
        scattered       = Rayf(hitData.p, target - hitData.p, rayIn.time());
        attenuation     = albedo;
        return true;
    }

    hq::math::Vector3f albedo;
};

class Metal : public Material
{
public:
    Metal(const hq::math::Vector3f& albedo, float roughness = 0.f)
        : albedo(albedo)
    {
        if (roughness < 1.f)
            this->roughness = roughness;
        else
            this->roughness = 1.f;
    }

    bool scatter(const hq::math::Rayf& rayIn, const HitData& hitData, hq::math::Vector3f& attenuation,
                 hq::math::Rayf& scattered) const override
    {
        using namespace hq::math;
        Vector3f reflected = reflect(rayIn.direction(), hitData.normal);
        scattered          = Rayf(hitData.p, reflected + roughness * RandomInUnitSphere());
        attenuation        = albedo;
        return (dot(scattered.direction(), hitData.normal) > 0.f);
    }

    hq::math::Vector3f albedo;
    float              roughness;
};

class Dielectric : public Material
{
public:
    Dielectric(float refIdx)
        : refIdx(refIdx)
    {
    }

    bool scatter(const hq::math::Rayf& rayIn, const HitData& hitData, hq::math::Vector3f& attenuation,
                 hq::math::Rayf& scattered) const override
    {
        using namespace hq::math;
        Vector3f outwardNormal;
        Vector3f reflected = reflect(rayIn.direction(), hitData.normal);
        float    niOverNt;
        attenuation = Vector3f(1.f, 1.f, 1.f);
        Vector3f refracted;
        float    reflectProb;
        float    cosine;
        if (dot(rayIn.direction(), hitData.normal) > 0.f)
        {
            outwardNormal = -hitData.normal;
            niOverNt      = refIdx;
            cosine        = refIdx * dot(rayIn.direction(), hitData.normal);
        }
        else
        {
            outwardNormal = hitData.normal;
            niOverNt      = 1.f / refIdx;
            cosine        = -dot(rayIn.direction(), hitData.normal);
        }
        if (refract(rayIn.direction(), outwardNormal, niOverNt, refracted))
        {
            reflectProb = schlick(cosine, refIdx);
        }
        else
        {
            scattered   = Rayf(hitData.p, reflected);
            reflectProb = 1.f;
        }
        if (hq::rand01() < reflectProb)
        {
            scattered = Rayf(hitData.p, reflected);
        }
        else
        {
            scattered = Rayf(hitData.p, refracted);
        }

        return true;
    }

    float schlick(float cosine, float refIdx) const
    {
        float r0 = (1.f - refIdx) / (1 + refIdx);
        r0       = r0 * r0;
        return r0 + (1 - r0) * std::pow((1.f - cosine), 5.f);
    }

    float refIdx;
};
