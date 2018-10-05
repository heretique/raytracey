#pragma once

#include "aabb.h"
#include "vector.h"

#undef M_PI
#define M_PI (3.14159265358979f)
float          rand01();
math::Vector3f RandomInUnitSphere();
math::Vector3f RandomInUnitDisk();
math::Vector3f reflect(const math::Vector3f& v, const math::Vector3f& n);
bool           refract(const math::Vector3f& v, const math::Vector3f& n, float niOverNt, math::Vector3f& refracted);
bool           cmpf(float a, float b, float epsilon);
math::AABBf    sphereBbox(const math::Vector3f& center, const float radius);
math::AABBf    surroundingBbox(const math::AABBf& bbox1, const math::AABBf& bbox2);
