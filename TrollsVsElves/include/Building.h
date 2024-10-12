#ifndef BUILDING_H
#define BUILDING_H

#include "structs.h"
#include "UIUtils.h"
#include "Building.h"
#include <string>
#include <vector>

class Player; // forward declaration to get around circular dependency

enum BuildStage { NONE = 0, GHOST, SCHEDULED, IN_PROGRESS, FINISHED };
enum BuildingType { CASTLE = 0, ROCK, HALL, SHOP };

struct Building {
    Cube cube;
    BuildStage buildStage;

    Color ghostColor;
    Color inProgressColor;
    Color selectedColor;
    Color targetColor;

    float buildTime;
    float buildTimer;

    bool selected;
    Cylinder rallyPoint;

    Player* owner;
    BuildingType buildingType;
    std::string actionId;
    std::vector<std::string> previousActionIds;
    bool sold;
};

void building_init(Building* building, Cube cube, BuildingType buildingType, Player* owner);

void building_draw(Building* building);
void building_update(Building* building);

void building_schedule_build(Building* building);
void building_start_build(Building* building);
void building_finish_build(Building* building);

// void building_set_position(Building* building, Vector3 position);
// Vector3 building_get_position(Building* building);
// Cube* building_get_cube(Building* building);

// Color building_get_ghost_color(Building* building);

void building_select(Building* building);
void building_deselect(Building* building);
// bool building_is_selected(Building* building);

// Cylinder building_get_rally_point(Building* building);
// void building_set_rally_point(Building* building, Vector3 point);

#endif
