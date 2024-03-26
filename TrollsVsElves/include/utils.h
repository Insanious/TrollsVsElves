#pragma once

#include "raylib.h"
#include <cstdio>
#include <string>

inline void printVector2(std::string prefix, Vector2 vec)
{
    printf("%s: %f, %f\n", prefix.c_str(), vec.x, vec.y);
}

inline void printVector3(std::string prefix, Vector3 vec)
{
    printf("%s: %f, %f, %f\n", prefix.c_str(), vec.x, vec.y, vec.z);
}

inline float nearestIncrement(float value, float increment)
{
    float truncated = float(int(value) / int(increment));
    return truncated * increment;
}
