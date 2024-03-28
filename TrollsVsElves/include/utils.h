#pragma once

#include "raylib.h"
#include <cstdio>
#include <string>
#include <vector>

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

template <class T>
inline void swapAndPop(std::vector<T>& vector, const T& element)
{
    if (!vector.size() || !element) return;

    auto it = find(vector.begin(), vector.end(), element);
    if (it == vector.end()) return;

    std::iter_swap(it, vector.end() - 1);
    vector.pop_back();
}
