#include "Building.h"

Building::Building(Cube cube, BuildingType buildingType)
{
    buildStage = GHOST;

    ghostColor = { 0, 121, 241, 100 };
    inProgressColor = { 255, 255, 255, 100 };

    this->cube = cube;
    this->buildingType = buildingType;
    cube.color = ghostColor;

    selected = false;
    sold = false;

    buildTime = 0.2f;
    buildTimer = 0.f;

    rallyPoint = Cylinder(Vector3Zero(), 0.8f, 20.f, 8, { 255, 255, 255, 128 });

    switch (buildingType)
    {
        case ROCK:      targetColor = Color{ 100, 100, 100, 255 };  break;
        case CASTLE:    targetColor = BEIGE;                        break;
        case HALL:      targetColor = BLUE;                         break;
        case SHOP:      targetColor = SKYBLUE;                      break;
    }
    switch (buildingType)
    {
        case ROCK:      this->actionId = "rock0";   break;
        case CASTLE:    this->actionId = "castle0"; break;
        case HALL:      this->actionId = "hall0";   break;
        case SHOP:      this->actionId = "shop0";   break;
    }

    Vector3 targetColorHSL = ColorToHSV(targetColor);
    selectedColor = ColorFromHSV(targetColorHSL.x, targetColorHSL.y, targetColorHSL.z * 0.8f);
}

Building::~Building() {}

void Building::draw()
{
    drawCube(cube);

    if (selected && !Vector3Equals(rallyPoint.position, cube.position)) // This might be a temporary solution TODO: later
        drawCylinder(rallyPoint);
}

void Building::update()
{
    if (buildStage == IN_PROGRESS)
    {
        buildTimer += GetFrameTime();
        float adjusted = buildTimer/buildTime;
        cube.color = lerpColor(inProgressColor, targetColor, adjusted);

        if (buildTimer >= buildTime)
        {
            cube.color = selected ? selectedColor : targetColor;
            buildTimer = 0.f;
            buildStage = FINISHED;
        }
    }
}

void Building::scheduleBuild()
{
    assert(buildStage == GHOST); // sanity check

    buildStage = SCHEDULED;
    cube.color = inProgressColor;
    rallyPoint.position = { cube.position.x, cube.position.y, cube.position.z };
}

void Building::build()
{
    assert(buildStage == SCHEDULED); // sanity check

    buildStage = IN_PROGRESS;
}

void Building::setPosition(Vector3 position)
{
    cube.position = position;
}

Vector3 Building::getPosition()
{
    return cube.position;
}

Cube& Building::getCube()
{
    return cube;
}

Color Building::getGhostColor()
{
    return ghostColor;
}

void Building::select()
{
    selected = true;
    if (buildStage != IN_PROGRESS)
        cube.color = selectedColor;
}

void Building::deselect()
{
    selected = false;
    if (buildStage != IN_PROGRESS)
        cube.color = targetColor;
}

bool Building::isSelected()
{
    return selected;
}

Cylinder Building::getRallyPoint()
{
    return rallyPoint;
}

void Building::setRallyPoint(Vector3 point)
{
    rallyPoint.position = point;
}
