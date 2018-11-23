#pragma once

#include <Hq/Math/Vector.h>
#include <memory>
#include "FastNoise/FastNoise.h"

class Texture
{
public:
    virtual ~Texture() {}
    virtual hq::math::Vector3f value(float u, float v, const hq::math::Vector3f& p) const = 0;
};

using TexturePtr = std::shared_ptr<Texture>;

class ColorTexture : public Texture
{
public:
    ColorTexture() {}
    ColorTexture(hq::math::Vector3f color)
        : color(color)
    {
    }

    hq::math::Vector3f value(float /*u*/, float /*v*/, const hq::math::Vector3f& /*p*/) const override
    {
        return color;
    }

    hq::math::Vector3f color;
};

class CheckerTexture : public Texture
{
public:
    CheckerTexture() {}
    CheckerTexture(TexturePtr t1, TexturePtr t2)
        : evenTexture(t1)
        , oddTexture(t2)
    {
    }

    hq::math::Vector3f value(float u, float v, const hq::math::Vector3f& p) const override
    {
        float sines = std::sin(10 * p.x) * std::sin(10 * p.y) * std::sin(10 * p.z);
        if (sines < 0)
            return oddTexture->value(u, v, p);
        else
            return evenTexture->value(u, v, p);
    }

    TexturePtr evenTexture;
    TexturePtr oddTexture;
};

class Perlin
{
public:
    float noise(const hq::math::Vector3f& p) const
    {
        float u = p.x - std::floorf(p.x);
        float v = p.y - std::floorf(p.y);
        float w = p.z - std::floorf(p.z);
        int   i = int(4.f * p.x) & 255;
        int   j = int(4.f * p.y) & 255;
        int   k = int(4.f * p.z) & 255;
    }
};

class NoiseTexture : public Texture
{
public:
    NoiseTexture() {}
    NoiseTexture(FastNoise::NoiseType noiseType)
    {
        noise.SetNoiseType(noiseType);
    }
    hq::math::Vector3f value(float u, float v, const hq::math::Vector3f& p) const override
    {
        using hq::math::Vector3f;
        return Vector3f(1.f, 1.f, 1.f) * ((noise.GetNoise(p.x, p.y, p.z) + 1.f) / 2.f);
    }

    FastNoise noise;
};
