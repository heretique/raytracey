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

bool cmpf(float a, float b, float epsilon)
{
    return fabs(a - b) < epsilon;
}

math::AABBf sphereBbox(const math::Vector3f& center, const float radius)
{
    using namespace math;
    return AABBf(center - Vector3f(radius, radius, radius), center + Vector3f(radius, radius, radius));
}

math::AABBf surroundingBbox(const math::AABBf& bbox1, const math::AABBf& bbox2)
{
    using namespace math;
    Vector3f small(
        fminf(bbox1.min().x, bbox2.min().x), fminf(bbox1.min().y, bbox2.min().y), fminf(bbox1.min().z, bbox2.min().z));
    Vector3f big(
        fmaxf(bbox1.max().x, bbox2.max().x), fmaxf(bbox1.max().y, bbox2.max().y), fmaxf(bbox1.max().z, bbox2.max().z));
    return AABBf(small, big);
}
