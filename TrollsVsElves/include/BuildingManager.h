#ifndef BUILDING_MANAGER_H
#define BUILDING_MANAGER_H

#include <vector>
#include <deque>
#include <unordered_map>

#include "utils.h"
#include "MapGenerator.h"
#include "Building.h"
#include "CameraManager.h"
#include "ActionsManager.h"
#include "UIUtils.h"

class Player; // forward declaration to get around circular depenedency

struct BuildingManager {
    Vector3 defaultBuildingSize;
    Color defaultBuildingColor;

    std::vector<Building> buildings;
    std::deque<Building> buildQueue;
    Building ghostBuilding;
    bool ghostBuildingIsColliding;

    std::unordered_map<std::string, unsigned> unlockedActions;

    size_t selectedBuildingIndex;
};

void building_manager_init(BuildingManager* manager, Vector3 defaultBuildingSize, Color defaultBuildingColor);
void building_manager_cleanup(BuildingManager* manager);

void building_manager_draw(BuildingManager* manager);
void building_manager_draw_ui_buttons(BuildingManager* manager, ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine);
void building_manager_resolve_action(BuildingManager* manager, Building* building, ActionNode* action);
void building_manager_update(BuildingManager* manager);

Building* building_manager_raycast(BuildingManager* manager);
void building_manager_remove_selected_building(BuildingManager* manager);

Building* building_manager_yield_queue(BuildingManager* manager);
bool building_manager_queue_empty(BuildingManager* manager);
Building* building_manager_queue_front(BuildingManager* manager);
void building_manager_clear_queue(BuildingManager* manager);

void building_manager_create_debug_building(BuildingManager* manager, Vector2i index, BuildingType type);

void building_manager_update_ghost_building(BuildingManager* manager);
void building_manager_create_ghost_building(BuildingManager* manager, BuildingType buildingType, Player* player);
void building_manager_clear_ghost_building(BuildingManager* manager);
void building_manager_schedule_ghost_building(BuildingManager* manager);
bool building_manager_can_schedule_ghost_building(BuildingManager* manager);
Building* building_manager_get_ghost_building(BuildingManager* manager);
bool building_manager_ghost_building_exists(BuildingManager* manager);
bool building_manager_ghost_building_is_colliding(BuildingManager* manager);

Building* building_manager_get_selected_building(BuildingManager* manager);
void building_manager_select(BuildingManager* manager, Building* building);
void building_manager_deselect(BuildingManager* manager);

void building_manager_recruit(BuildingManager* manager, Building* building);
bool building_manager_can_promote_to(BuildingManager* manager, const char* id);
void building_manager_promote(BuildingManager* manager, Building* building, const char* id);

// void building_manager_sell(BuildingManager* manager, Building* building);

// class BuildingManager
// {
// private:
//     Vector3 defaultBuildingSize;
//     Color defaultBuildingColor;

//     std::vector<Building*> buildings;
//     std::deque<Building*> buildQueue;
//     Building* ghostBuilding;
//     bool ghostBuildingIsColliding;

//     std::unordered_map<std::string, unsigned> unlockedActions;

//     void updateGhostBuilding();

//     template<typename Container>
//     bool isColliding(const Container& buildings, Building* targetBuilding);

// public:
//     Building* selectedBuilding;

//     BuildingManager() = delete;
//     BuildingManager(Vector3 defaultBuildingSize, Color defaultBuildingColor);
//     ~BuildingManager();

//     void draw();
//     void drawBuildingUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine);
//     void resolveBuildingAction(Building* building, ActionNode& action);
//     void update();

//     Building* raycastToBuilding();
//     void removeBuilding(Building* building);

//     Building* yieldBuildQueue();
//     Building* buildQueueFront();
//     void clearBuildQueue();

//     void createDebugBuilding(Vector2i index, BuildingType type);
//     void createNewGhostBuilding(BuildingType buildingType, Player* player);
//     void clearGhostBuilding();
//     void scheduleGhostBuilding();
//     bool canScheduleGhostBuilding();
//     Building* getGhostBuilding();
//     bool ghostBuildingExists();

//     void select(Building* building);
//     void deselect();

//     void recruit(Building* building);
//     bool canPromoteTo(std::string id);
//     void promote(Building* building, std::string id);

//     void sell(Building* building);
// };

#endif
