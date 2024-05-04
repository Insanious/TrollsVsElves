#ifndef BUILDING_MANAGER_H
#define BUILDING_MANAGER_H

#include <vector>
#include <deque>
#include <unordered_set>

#include "utils.h"
#include "MapGenerator.h"
#include "Building.h"
#include "Entity.h"
#include "CameraManager.h"

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

    std::vector<Entity*> entities;

    std::map<BuildingType, AdvancementTree*> advancementTrees;
    std::unordered_set<std::string> unlockedAdvancements;

    Player* player;

    void updateGhostBuilding();

    template<typename Container>
    bool isColliding(const Container& buildings, Building* targetBuilding);

public:
    BuildingManager() = delete;
    BuildingManager(Vector3 defaultBuildingSize, Color defaultBuildingColor);
    ~BuildingManager();

    void draw();
    void drawBuildingUIButtons(Building* building, ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine);
    void checkKeyboardPresses(Building* building, std::vector<AdvancementNode*> children);
    void update();

    void setPlayer(Player* player);

    Building* raycastToBuilding();
    void removeBuilding(Building* building);

    Building* yieldBuildQueue();
    Building* buildQueueFront();
    void clearBuildQueue();

    void createDebugBuilding(Vector2i index, BuildingType type);
    void createNewGhostBuilding(BuildingType buildingType);
    void clearGhostBuilding();
    void scheduleGhostBuilding();
    bool canScheduleGhostBuilding();
    Building* getGhostBuilding();
    bool ghostBuildingExists();

    std::vector<Entity*> getEntities();

    void recruit(Building* building);
    bool canPromoteTo(AdvancementNode* promotion);
    void promote(Building* building, AdvancementNode* promotion);
};

#endif
