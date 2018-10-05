#pragma once

#include "ray.h"
#include "utils.h"
#include "vector.h"

class Camera
{
public:
    Camera(math::Vector3f eye, math::Vector3f lookAt, math::Vector3f up, float vFov, float aspect, float aperture,
           float focusDistance, float timeStart, float timeEnd)
    {
        using namespace math;

        this->timeStart  = timeStart;
        this->timeEnd    = timeEnd;
        lensRadius       = aperture / 2;
        float theta      = vFov * M_PI / 180.0f;
        float halfHeight = std::tanf(theta / 2);
        float halfWidth  = aspect * halfHeight;
        origin           = eye;
        w                = normalize(eye - lookAt);
        u                = normalize(cross(up, w));
        v                = cross(w, u);
        lowerLeft        = origin - halfWidth * focusDistance * u - halfHeight * focusDistance * v - focusDistance * w;
        horizontal       = 2 * halfWidth * focusDistance * u;
        vertical         = 2 * halfHeight * focusDistance * v;
    }

    math::Rayf getRay(float s, float t)
    {
        math::Vector3f rd     = lensRadius * RandomInUnitDisk();
        math::Vector3f offset = u * rd.x + v * rd.y;
        float          time   = timeStart + rand01() * (timeStart - timeEnd);
        return math::Rayf(origin + offset, lowerLeft + s * horizontal + t * vertical - origin - offset, time);
    }

public:
    math::Vector3f origin;
    math::Vector3f lowerLeft;
    math::Vector3f horizontal;
    math::Vector3f vertical;
    math::Vector3f u, v, w;
    float          lensRadius;
    float          timeStart;
    float          timeEnd;
};
