#include "Resource.h"

Resource::Resource(Cube cube)
{
    this->cube = cube;

    selected = false;

    defaultColor = DARKGREEN;
    Vector3 defaultColorHSL = ColorToHSV(defaultColor);
    selectedColor = ColorFromHSV(defaultColorHSL.x, defaultColorHSL.y, defaultColorHSL.z - 0.2f);

    this->cube.color = defaultColor;
}

Resource::~Resource()
{
}

void Resource::draw()
{
    drawCube(cube);
}

Cube& Resource::getCube()
{
    return cube;
}