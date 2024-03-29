#pragma once

#include "raylib.h"
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>

inline void printVector2(std::string prefix, Vector2 vec)
{
    printf("%s: %f, %f\n", prefix.c_str(), vec.x, vec.y);
}

inline void printVector3(std::string prefix, Vector3 vec)
{
    printf("%s: %f, %f, %f\n", prefix.c_str(), vec.x, vec.y, vec.z);
}

inline void printColor(std::string prefix, Color color)
{
    printf("%s: %u, %u, %u, %u\n", prefix.c_str(), color.r, color.g, color.b, color.a);
}

inline float nearestIncrement(float value, float increment)
{
    float truncated = float(int(value) / int(increment));
    return truncated * increment;
}

template <class T>
inline void swapAndPop(std::vector<T>& vector, const T& element)
{
    if (!vector.size() || !element) return;

    auto it = find(vector.begin(), vector.end(), element);
    if (it == vector.end()) return;

    std::iter_swap(it, vector.end() - 1);
    vector.pop_back();
}

inline Color lerpColor(Color c1, Color c2, float amount)
{
    return {
        static_cast<unsigned char>(c1.r + amount * (c2.r - c1.r)),
        static_cast<unsigned char>(c1.g + amount * (c2.g - c1.g)),
        static_cast<unsigned char>(c1.b + amount * (c2.b - c1.b)),
        static_cast<unsigned char>(c1.a + amount * (c2.a - c1.a)),
    };
}
