#pragma once

#include "raylib.h"
#include "raymath.h"
#include "utils.h"
#include <cmath>
#include <cassert>
#include <string>

struct Cube
{
    Vector3 position = Vector3Zero();
    Vector3 size = Vector3Zero();
    Color color = BLANK;

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
    Color color = BLANK;

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
    float radius = 2.f;
    float height = 3.f;
    int slices = 16;
    int rings = 4;
    Color color = BLANK;

    Capsule() {};

    Capsule(float _radius, float _height)
        : radius(_radius), height(_height) {};
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

    bool operator==(const Vector2i& rhs) const
    {
        return this->x == rhs.x && this->y == rhs.y;
    }
};

inline void printVector2i(std::string prefix, Vector2i vec)
{
    printf("%s: %d, %d\n", prefix.c_str(), vec.x, vec.y);
}

struct Item
{
    std::string name = "";

    Item() {};
    Item(std::string _name): name(_name) {};
};
