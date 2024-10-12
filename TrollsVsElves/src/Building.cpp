#include "Building.h"
#include "Player.h"

void building_init(Building* building, Cube cube, BuildingType buildingType, Player* owner) {
    building->buildStage = BuildStage::NONE;

    building->ghostColor = (Color){ 0, 121, 241, 100 };
    building->inProgressColor = (Color){ 255, 255, 255, 100 };

    building->cube = cube;
    building->buildingType = buildingType;
    building->owner = owner;
    building->cube.color = building->ghostColor;

    building->selected = false;
    building->sold = false;

    building->buildTime = 0.2f;
    building->buildTimer = 0.f;

    building->rallyPoint = Cylinder(Vector3Zero(), 0.8f, 20.f, 8, (Color){ 255, 255, 255, 128 });

    // Set target color based on building type
    switch (buildingType) {
        case ROCK:      building->targetColor = (Color){ 100, 100, 100, 255 };  break;
        case CASTLE:    building->targetColor = BEIGE;                          break;
        case HALL:      building->targetColor = BLUE;                           break;
        case SHOP:      building->targetColor = SKYBLUE;                        break;
    }

    // Assign actionId
    switch (buildingType) {
        case ROCK:      building->actionId = "rock0";   break;
        case CASTLE:    building->actionId = "castle0"; break;
        case HALL:      building->actionId = "hall0";   break;
        case SHOP:      building->actionId = "shop0";   break;
    }

    Vector3 targetColorHSL = ColorToHSV(building->targetColor);
    building->selectedColor = ColorFromHSV(targetColorHSL.x, targetColorHSL.y, targetColorHSL.z * 0.8f);
}

void building_draw(Building* building) {
    drawCube(building->cube);

    if (building->selected && !Vector3Equals(building->rallyPoint.position, building->cube.position)) {
        drawCylinder(building->rallyPoint);
    }
}

void building_update(Building* building) {
    if (building->buildStage == IN_PROGRESS) {
        building->buildTimer += GetFrameTime();
        float adjusted = building->buildTimer / building->buildTime;
        building->cube.color = lerpColor(building->inProgressColor, building->targetColor, adjusted);

        if (building->buildTimer >= building->buildTime)
            building_finish_build(building);
    }
}

void building_schedule_build(Building* building) {
    assert(building->buildStage == GHOST);
    building->buildStage = SCHEDULED;
    building->cube.color = building->inProgressColor;
    building->rallyPoint.position = building->cube.position;
}

void building_start_build(Building* building) {
    assert(building->buildStage == SCHEDULED);
    building->buildStage = IN_PROGRESS;
}

void building_finish_build(Building* building) {
    assert(building->buildStage == IN_PROGRESS);
    building->cube.color = building->selected ? building->selectedColor : building->targetColor;
    building->buildTimer = 0.f;
    building->buildStage = FINISHED;
}

void building_select(Building* building) {
    building->selected = true;
    if (building->buildStage != IN_PROGRESS)
        building->cube.color = building->selectedColor;
}

void building_deselect(Building* building) {
    building->selected = false;
    if (building->buildStage != IN_PROGRESS)
        building->cube.color = building->targetColor;
}