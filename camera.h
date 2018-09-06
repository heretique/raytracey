#pragma once

#include "ray.h"
#include "vector.h"

class Camera
{
public:
    Camera()
        : lowerLeft(math::Vector3f(-2.f, -1.f, -1.f))
        , horizontal(math::Vector3f(4.f, 0.f, 0.f))
        , vertical(math::Vector3f(0.f, 2.f, 0.f))
        , origin(math::Vector3f(0.f, 0.f, 0.f))
    {
    }

    math::Rayf getRay(float u, float v)
    {
        return math::Rayf(origin, lowerLeft + u * horizontal + v * vertical - origin);
    }

    math::Vector3f origin;
    math::Vector3f lowerLeft;
    math::Vector3f horizontal;
    math::Vector3f vertical;
};
