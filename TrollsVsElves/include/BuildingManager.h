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

class BuildingManager
{
private:
    Vector3 defaultBuildingSize;
    Color defaultBuildingColor;

    std::vector<Building*> buildings;
    std::deque<Building*> buildQueue;
    Building* ghostBuilding;
    bool ghostBuildingIsColliding;

    std::unordered_map<std::string, unsigned> unlockedActions;

    void updateGhostBuilding();

    template<typename Container>
    bool isColliding(const Container& buildings, Building* targetBuilding);

public:
    Building* selectedBuilding;

    BuildingManager() = delete;
    BuildingManager(Vector3 defaultBuildingSize, Color defaultBuildingColor);
    ~BuildingManager();

    void draw();
    void drawBuildingUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine);
    void resolveBuildingAction(Building* building, ActionNode& action);
    void update();
    void updateBuildings(std::vector<Building*>& buildings);

    Building* raycastToBuilding();
    void removeBuilding(Building* building);

    Building* yieldBuildQueue();
    Building* buildQueueFront();
    void clearBuildQueue();

    void createDebugBuilding(Vector2i index, BuildingType type);
    void createNewGhostBuilding(BuildingType buildingType, Player* player);
    void clearGhostBuilding();
    void scheduleGhostBuilding();
    bool canScheduleGhostBuilding();
    Building* getGhostBuilding();
    bool ghostBuildingExists();

    void select(Building* building);
    void deselect();

    void recruit(Building* building);
    bool canPromoteTo(std::string id);
    void promote(Building* building, std::string id);
};

#endif
