#pragma once

#include "ray.h"
#include "vector.h"

namespace math
{
template <typename T>
class AABB
{
public:
    typedef Vector<T, 3> VectorType;
    typedef Ray<T>       RayType;

    AABB() {}
    AABB(VectorType min, VectorType max)
        : _min(min)
        , _max(max)
    {
    }

    VectorType min() const
    {
        return _min;
    }
    VectorType max() const
    {
        return _max;
    }

    bool hit(const RayType& rayIn, float tMin, float tMax) const
    {
        for (int i = 0; i < 3; ++i)
        {
            float invD = 1.f / rayIn.direction()[i];
            float t0   = (_min[i] - rayIn.origin()[i]) * invD;
            float t1   = (_max[i] - rayIn.origin()[i]) * invD;
            if (invD < 0.f)
                std::swap(t0, t1);
            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;
            if (tMax <= tMin)
                return false;
        }
        return true;
    }

private:
    VectorType _min;
    VectorType _max;
};

using AABBf = AABB<float>;

}  // namespace math
