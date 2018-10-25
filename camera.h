#pragma once

#include <Hq/Math/Ray.h>
#include <Hq/Math/Utils.h>
#include <Hq/Math/Vector.h>
#include <Hq/Rng.h>

class Camera
{
public:
    Camera(hq::math::Vector3f eye, hq::math::Vector3f lookAt, hq::math::Vector3f up, float vFov, float aspect,
           float aperture, float focusDistance, float timeStart, float timeEnd)
    {
        using namespace hq::math;

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

    hq::math::Rayf getRay(float s, float t)
    {
        hq::math::Vector3f rd     = lensRadius * hq::math::RandomInUnitDisk();
        hq::math::Vector3f offset = u * rd.x + v * rd.y;
        float              time   = timeStart + hq::rand01() * (timeStart - timeEnd);
        return hq::math::Rayf(origin + offset, lowerLeft + s * horizontal + t * vertical - origin - offset, time);
    }

public:
    hq::math::Vector3f origin;
    hq::math::Vector3f lowerLeft;
    hq::math::Vector3f horizontal;
    hq::math::Vector3f vertical;
    hq::math::Vector3f u, v, w;
    float              lensRadius;
    float              timeStart;
    float              timeEnd;
};
