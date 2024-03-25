#pragma once

#include "raylib.h"
#include "raymath.h"
#include <cstdio>
#include <string>

struct Cube
{
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    Vector3 size = { 0.0f, 0.0f, 0.0f };
    Color color = WHITE;

    Cube(Vector3 _position, Vector3 _size, Color _color): position(_position), size(_size), color(_color) {};
};

struct Vector2i
{
    int x = 0;
    int y = 0;
};

inline BoundingBox getCubeBoundingBox(Cube cube)
{
    Vector3 halfSize = Vector3Scale(cube.size, 0.5f);
    return (BoundingBox) { Vector3Add(cube.position, halfSize), Vector3Subtract(cube.position, halfSize) };
}

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

inline void drawCube(Cube* cube)
{
    DrawCubeV(cube->position, cube->size, cube->color);
    DrawCubeWiresV(cube->position, cube->size, GRAY);
}

// Cube* raycastToClosestCube(Camera camera, std::vector<Cube*> cubes)
// {
//     Ray ray = GetMouseRay(GetMousePosition(), camera);
//     float closestCollisionDistance = std::numeric_limits<float>::infinity();;
//     Cube* closestCube = nullptr;

//     for (Cube* cube: cubes)
//     {
//         RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(*cube));

//         if (collision.hit && collision.distance < closestCollisionDistance) {
//             closestCollisionDistance = collision.distance;
//             closestCube = cube;
//         }
//     }

//     return closestCube;
// }