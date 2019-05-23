#pragma once

#include <Hq/Math/Vector.h>
#include <memory>
#include "FastNoise/FastNoise.h"
#include <Hq/Math/Utils.h>
#include <assert.h>

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
        (void)u;
        (void)v;
        using hq::math::Vector3f;
        return Vector3f(1.f, 1.f, 1.f) * ((noise.GetNoise(p.x, p.y, p.z) + 1.f) / 2.f);
    }

    FastNoise noise;
};

class ImageTexture : public Texture
{
public:
    ImageTexture() = delete;
    // data is always 4 channels (RGBA)
    ImageTexture(const unsigned char* data, int width, int height)
        : data(data)
        , width(width)
        , height(height)
    {
    }

    hq::math::Vector3f value(float u, float v, const hq::math::Vector3f& p) const override
    {
        (void)p;
        assert(0 <= u <= 1);
        assert(0 <= v <= 1);
        using vec3 = hq::math::Vector3f;
        using namespace hq::math;
        int   i = clamp(int(u * width), 0, width - 1);
        int   j = clamp(int((1.f - v) * height), 0, height);
        float r = int(data[4 * (i + width * j)]) / 255.f;
        float g = int(data[4 * (i + width * j) + 1]) / 255.f;
        float b = int(data[4 * (i + width * j) + 2]) / 255.f;
        return vec3(r, g, b);
    }

    const unsigned char* data;
    int                  width;
    int                  height;
};
