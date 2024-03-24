#include "Building.h"

Building::Building()
{
}

Building::Building(Vector3 position, Vector3 size, Color color)
{
    defaultColor = color;
    cube = new Cube(position, size, color);
}

Building::~Building()
{
    delete cube;
}

void Building::draw()
{
    if (cube) drawCube(cube);
}

void Building::setPosition(Vector3 position)
{
    cube->position = position;
}

Vector3 Building::getPosition()
{
    return cube->position;
}

Cube* Building::getCube()
{
    return cube;
}

void Building::select()
{
    cube->color = RED;
}

void Building::deselect()
{
    cube->color = defaultColor;
}