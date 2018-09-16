#pragma once

#include "hitable.h"
#include "utils.h"

class Material
{
public:
    virtual ~Material() {}
    virtual bool scatter(const math::Rayf& rayIn, const HitData& hitData, math::Vector3f& attenuation,
                         math::Rayf& scattered) const = 0;
};

class Lambertian : public Material
{
public:
    Lambertian(const math::Vector3f& albedo)
        : albedo(albedo)
    {
    }

    bool scatter(const math::Rayf&, const HitData& hitData, math::Vector3f& attenuation,
                 math::Rayf& scattered) const override
    {
        math::Vector3f target = hitData.p + hitData.normal + RandomInUnitSphere();
        scattered             = math::Rayf(hitData.p, target - hitData.p);
        attenuation           = albedo;
        return true;
    }

    math::Vector3f albedo;
};

class Metal : public Material
{
public:
    Metal(const math::Vector3f& albedo, float roughness = 0.f)
        : albedo(albedo)
    {
        if (roughness < 1.f)
            this->roughness = roughness;
        else
            this->roughness = 1.f;
    }

    bool scatter(const math::Rayf& rayIn, const HitData& hitData, math::Vector3f& attenuation,
                 math::Rayf& scattered) const override
    {
        math::Vector3f reflected = reflect(rayIn.direction(), hitData.normal);
        scattered                = math::Rayf(hitData.p, reflected + roughness * RandomInUnitSphere());
        attenuation              = albedo;
        return (math::dot(scattered.direction(), hitData.normal) > 0.f);
    }

    math::Vector3f albedo;
    float          roughness;
};

class Dielectric : public Material
{
public:
    Dielectric(float refIdx)
        : refIdx(refIdx)
    {
    }

    bool scatter(const math::Rayf& rayIn, const HitData& hitData, math::Vector3f& attenuation,
                 math::Rayf& scattered) const override
    {
        math::Vector3f outwardNormal;
        math::Vector3f reflected = reflect(rayIn.direction(), hitData.normal);
        float          niOverNt;
        attenuation = math::Vector3f(1.f, 1.f, 1.f);
        math::Vector3f refracted;
        float          reflectProb;
        float          cosine;
        if (math::dot(rayIn.direction(), hitData.normal) > 0.f)
        {
            outwardNormal = -hitData.normal;
            niOverNt      = refIdx;
            cosine        = refIdx * math::dot(rayIn.direction(), hitData.normal);
        }
        else
        {
            outwardNormal = hitData.normal;
            niOverNt      = 1.f / refIdx;
            cosine        = -math::dot(rayIn.direction(), hitData.normal);
        }
        if (refract(rayIn.direction(), outwardNormal, niOverNt, refracted))
        {
            reflectProb = schlick(cosine, refIdx);
        }
        else
        {
            scattered   = math::Rayf(hitData.p, reflected);
            reflectProb = 1.f;
        }
        if (rand01() < reflectProb)
        {
            scattered = math::Rayf(hitData.p, reflected);
        }
        else
        {
            scattered = math::Rayf(hitData.p, refracted);
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
