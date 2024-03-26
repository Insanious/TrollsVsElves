#include "Building.h"

Building::Building()
{
}

Building::Building(Vector3 position, Vector3 size, Color color)
{
    cube = new Cube(position, size, color);
    buildStage = GHOST;
    defaultColor = color;
    sold = false;
    level = 1;
}

Building::~Building()
{
    delete cube;
}

void Building::build()
{
    printf("Building::build()\n");
    assert(buildStage == GHOST); // sanity check

    buildStage = IN_PROGRESS;
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

void Building::sell()
{
    sold = true;
}

bool Building::isSold()
{
    return sold;
}

void Building::upgrade()
{
    level += 1;
}

int Building::getLevel()
{
    return level;
}