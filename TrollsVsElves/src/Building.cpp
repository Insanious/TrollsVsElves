#include "Building.h"

Building::Building(Cube cube, BuildingType buildingType)
{
    buildStage = GHOST;

    ghostColor = { 0, 121, 241, 100 };
    inProgressColor = { 255, 255, 255, 100 };

    selected = false;
    sold = false;
    level = 1;
    canRecruit = true;
    recruiting = false;

    buildTime = 0.1f;
    // buildTime = 2.f;
    buildTimer = 0.f;

    this->cube = cube;
    this->buildingType = buildingType;
    cube.color = ghostColor;

    targetColor = buildingType == ROCK ? Color{ 60, 60, 60, 255 } : BEIGE;
    Vector3 targetColorHSL = ColorToHSV(targetColor);
    selectedColor = ColorFromHSV(targetColorHSL.x, targetColorHSL.y, targetColorHSL.z - 0.2f);

    rallyPoint = Cylinder(Vector3Zero(), 0.8f, 20.f, 8, { 255, 255, 255, 128 });
}

Building::~Building()
{
}

void Building::draw()
{
    drawCube(cube);

    if (selected && !Vector3Equals(rallyPoint.position, cube.position)) // This might be a temporary solution TODO: later
        drawCylinder(rallyPoint);
}

void Building::drawUIButtons(ImVec2 windowPadding, ImVec2 buttonSize)
{
    if (ImGui::Button("Upgrade", buttonSize))
        upgrade();
    ImGui::SameLine();
    if (ImGui::Button("Sell", buttonSize))
        sell();
    if (canRecruit)
    {
        if (ImGui::Button("Recruit", buttonSize))
            recruiting = true;
    }
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

bool Building::isRecruiting()
{
    return recruiting;
}

void Building::setRecruiting(bool value)
{
    recruiting = value;
}

int Building::getLevel()
{
    return level;
}

Cylinder Building::getRallyPoint()
{
    return rallyPoint;
}

void Building::setRallyPoint(Vector3 point)
{
    rallyPoint.position = point;
}
