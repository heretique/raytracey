#pragma once

#include "vector.h"

#undef M_PI
#define M_PI (3.14159265358979)
float          rand01();
math::Vector3f RandomInUnitSphere();
math::Vector3f RandomInUnitDisk();
math::Vector3f reflect(const math::Vector3f& v, const math::Vector3f& n);
bool           refract(const math::Vector3f& v, const math::Vector3f& n, float niOverNt, math::Vector3f& refracted);
