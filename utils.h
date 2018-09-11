#pragma once

#include "vector.h"

float          rand01();
math::Vector3f RandomInUnitSphere();
math::Vector3f RandomInUnitDisk();
math::Vector3f reflect(const math::Vector3f& v, const math::Vector3f& n);
bool refract(const math::Vector3f& v, const math::Vector3f& n, float niOverNt, math::Vector3f& refracted);
