#pragma once

#include "raylib.h"
#include "structs.h"
#include <cstdio>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <json/json.h>
#include <fstream>

inline void printVector2(std::string prefix, Vector2 vec)
{
    printf("%s: %f, %f\n", prefix.c_str(), vec.x, vec.y);
}

inline void printVector3(std::string prefix, Vector3 vec)
{
    printf("%s: %f, %f, %f\n", prefix.c_str(), vec.x, vec.y, vec.z);
}

inline void printBoundingBox(std::string prefix, BoundingBox box)
{
    printVector3(prefix, box.max);
    printVector3(prefix, box.min);
}

inline void printMatrix(std::string prefix, Matrix m)
{
    std::string out = "";
    out += std::to_string(m.m0) + ','; out += std::to_string(m.m4) + ','; out += std::to_string(m.m8) + ','; out += std::to_string(m.m12) + ',';
    out += '\n';
    out += std::to_string(m.m1) + ','; out += std::to_string(m.m5) + ','; out += std::to_string(m.m9) + ','; out += std::to_string(m.m13) + ',';
    out += '\n';
    out += std::to_string(m.m2) + ','; out += std::to_string(m.m6) + ','; out += std::to_string(m.m10) + ','; out += std::to_string(m.m14) + ',';
    out += '\n';
    out += std::to_string(m.m3) + ','; out += std::to_string(m.m7) + ','; out += std::to_string(m.m11) + ','; out += std::to_string(m.m15) + ',';
    out += '\n';
    printf("%s\n%s\n", prefix.c_str(), out.c_str());
}

inline void printColor(std::string prefix, Color color)
{
    printf("%s: %u, %u, %u, %u\n", prefix.c_str(), color.r, color.g, color.b, color.a);
}

inline void print2DBoolVector(std::string prefix, std::vector<std::vector<bool>> vec)
{
    std::string str = "";
    for (int y = 0; y < vec.size(); y++)
    {
        for (int x = 0; x < vec[y].size(); x++)
            str += vec[y][x] == false ? '0' : '1';

        str += '\n';
    }
    printf("%s\n%s\n", prefix.c_str(), str.c_str());
}

inline float nearestIncrementOld(float value, float increment)
{
    return float(int(value) / int(increment)) * increment;
}

inline float nearestIncrement(float value, float increment)
{
    return std::round(value / increment) * increment;
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

inline Json::Value parseJsonFile(std::string filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        printf("couldn't open file %s\n", filename.c_str());
        assert(file.is_open());
    }

    Json::Value json;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(file, json);
    file.close();

    if (!parsingSuccessful)
    {
        printf("couldn't parse json file %s\n", filename.c_str());
        assert(parsingSuccessful);
    }

    return json;
}
