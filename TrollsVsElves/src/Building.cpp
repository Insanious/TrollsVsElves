#include "Building.h"

Building::Building()
{
    buildStage = FLOATING;

    floatingColor = { 0, 121, 241, 100 };
    ghostColor = { 255, 255, 255, 100 };
    selectedColor = YELLOW;
    targetColor = GREEN;

    sold = false;
    level = 1;

    buildTime = 2.f;
    buildTimer = 0.f;
}

Building::~Building()
{
}

void Building::init(Cube cube, BUILDING_TYPE buildingType)
{
    this->cube = cube;
    this->buildingType = buildingType;

    buildStage = FLOATING;
    this->cube.color = floatingColor;

    this->targetColor = buildingType == ROCK ? (Color){ 60, 60, 60, 255 } : GOLD;
}

void Building::draw()
{
    drawCube(cube);
}

void Building::drawUIButtons(ImVec2 windowPadding, ImVec2 buttonSize)
{
    if (ImGui::Button("Upgrade", buttonSize))
        upgrade();
    ImGui::SameLine();
    if (ImGui::Button("Sell", buttonSize))
        sell();
}

void Building::update()
{
    if (buildStage == IN_PROGRESS)
    {
        buildTimer += GetFrameTime();
        float adjusted = buildTimer/buildTime;
        cube.color = lerpColor(ghostColor, targetColor, adjusted);

        if (buildTimer >= buildTime)
        {
            cube.color = targetColor;
            buildTimer = 0.f;
            buildStage = FINISHED;
        }
    }
}

void Building::scheduleBuild()
{
    assert(buildStage == FLOATING); // sanity check

    buildStage = GHOST;
    cube.color = ghostColor;
}

void Building::build()
{
    assert(buildStage == GHOST); // sanity check

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

void Building::select()
{
    if (buildStage != IN_PROGRESS)
        cube.color = selectedColor;
}

void Building::deselect()
{
    if (buildStage != IN_PROGRESS)
        cube.color = targetColor;
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

BUILD_STAGE Building::getBuildStage()
{
    return buildStage;
}

