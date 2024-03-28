#pragma once

#include "raylib.h"
#include "raymath.h"
#include "utils.h"
#include <cmath>

struct Cube
{
    Vector3 position = Vector3Zero();
    Vector3 size = Vector3Zero();
    Color color = WHITE;

    Cube(): position(), size(), color() {};

    Cube(Vector3 _position, Vector3 _size, Color _color)
        : position(_position), size(_size), color(_color) {};
};

inline BoundingBox getCubeBoundingBox(Cube cube)
{
    Vector3 halfSize = Vector3Scale(cube.size, 0.5f);
    return (BoundingBox) { Vector3Add(cube.position, halfSize), Vector3Subtract(cube.position, halfSize) };
}

inline float getCubeDiagonalLength(Cube cube)
{
    return sqrt(pow(cube.size.x, 2) + pow(cube.size.z, 2));
}

inline void drawCube(Cube cube)
{
    DrawCubeV(cube.position, cube.size, cube.color);
    DrawCubeWiresV(cube.position, cube.size, GRAY);
}

struct Capsule
{
    Vector3 startPos = Vector3Zero();
    Vector3 endPos = Vector3Zero();
    float radius = 0;
    int slices = 0;
    int rings = 0;
    Color color = WHITE;

    Capsule(): startPos(), endPos(), radius(), slices(), rings(), color() {};

    Capsule(Vector3 _startPos, Vector3 _endPos, float _radius, int _slices, int _rings, Color _color)
        : startPos(_startPos), endPos(_endPos), radius(_radius), slices(_slices), rings(_rings), color(_color) {};
};

inline void drawCapsule(Capsule capsule)
{
    DrawCapsule(capsule.startPos, capsule.endPos, capsule.radius, capsule.slices, capsule.rings, capsule.color);
    DrawCapsuleWires(capsule.startPos, capsule.endPos, capsule.radius, capsule.slices, capsule.rings, GRAY);
}

struct Vector2i
{
    int x = 0;
    int y = 0;
};