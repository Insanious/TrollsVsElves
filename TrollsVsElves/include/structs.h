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

    Cube(Vector3 _size): size(_size) {};

    Cube(Vector3 _position, Vector3 _size, Color _color)
        : position(_position), size(_size), color(_color) {};
};

inline BoundingBox getCubeBoundingBox(Cube cube, float scale = 1.0f)
{
    Vector3 halfSize = Vector3Scale(cube.size, (0.5f*scale));
    return { Vector3Subtract(cube.position, halfSize), Vector3Add(cube.position, halfSize) };
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

struct Cylinder
{
    Vector3 position = Vector3Zero();
    float radiusTop = 0;
    float radiusBottom = 0;
    float height = 0;
    int slices = 0;
    Color color = WHITE;

    Cylinder(): position(), radiusTop(), radiusBottom(), height(), slices(), color() {};

    Cylinder(Vector3 _position, float _radius, float _height, int _slices, Color _color) :
        position(_position),
        radiusTop(_radius),
        radiusBottom(_radius),
        height(_height),
        slices(_slices),
        color(_color) {};
};

inline void drawCylinder(Cylinder c)
{
    DrawCylinder(c.position, c.radiusTop, c.radiusBottom, c.height, c.slices, c.color);
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