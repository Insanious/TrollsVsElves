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

    simpleSignal::Signal<void(std::string)> promotionSignal;
    void onPromotion(std::string promotion);

    void updateGhostBuilding();

    template<typename Container>
    bool isColliding(const Container& buildings, Building* targetBuilding);

public:
    BuildingManager() = delete;
    BuildingManager(Vector3 defaultBuildingSize, Color defaultBuildingColor);
    ~BuildingManager();

    void draw();
    void update();

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

    void updateLockedPromotions();
};

#endif
