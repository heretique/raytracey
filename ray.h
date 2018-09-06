#pragma once

#include "vector.h"

namespace math
{
template <typename T>
class Ray
{
public:
    typedef Vector<T, 3> VectorType;

    Ray()
    {
    }
    Ray(const VectorType& origin, const VectorType& direction)
        : _origin(origin)
        , _direction(normalize(direction))
    {
    }

    VectorType origin() const
    {
        return _origin;
    }

    void setOrigin(const VectorType& origin)
    {
        _origin = origin;
    }

    VectorType direction() const
    {
        return _direction;
    }

    void setDirection(const VectorType& direction)
    {
        _direction = direction;
    }

    VectorType pointOnRay(float distance) const
    {
        return _origin + distance * _direction;
    }

private:
    VectorType _origin;
    VectorType _direction;
};

using Rayf = Ray<float>;
}
