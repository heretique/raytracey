#include "utils.h"
#include <ctime>
#include <random>

static auto                                  seed = time(nullptr);
static std::mt19937                          gRng(seed);
static std::uniform_real_distribution<float> gDistribution;

math::Vector3f RandomInUnitSphere()
{
    math::Vector3f p;
    do
    {
        p = 2.f * math::Vector3f(gDistribution(gRng), gDistribution(gRng), gDistribution(gRng)) -
            math::Vector3f(1.f, 1.f, 1.f);
    } while (lengthSquared(p) >= 1.f);
    return p;
}

float rand01()
{
    return gDistribution(gRng);
}

math::Vector3f reflect(const math::Vector3f& v, const math::Vector3f& n)
{
    return v - 2 * math::dot(v, n) * n;
}

bool refract(const math::Vector3f& v, const math::Vector3f& n, float niOverNt, math::Vector3f& refracted)
{
    math::Vector3f uv           = math::normalize(v);
    float          dt           = math::dot(uv, n);
    float          discriminant = 1.f - niOverNt * niOverNt * (1 - dt * dt);
    if (discriminant > 0.f)
    {
        refracted = niOverNt * (uv - n * dt) - n * std::sqrt(discriminant);
        return true;
    }
    else
        return false;
}

math::Vector3f RandomInUnitDisk()
{
    math::Vector3f p;
    do
    {
        p = 2.f * math::Vector3f(gDistribution(gRng), gDistribution(gRng), 0.f) - math::Vector3f(1.f, 1.f, 0.f);
    } while (math::dot(p, p) >= 1.f);
    return p;
}
