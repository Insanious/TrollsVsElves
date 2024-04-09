#ifndef BUILDING_MANAGER_H
#define BUILDING_MANAGER_H

#include <vector>
#include <deque>

#include "utils.h"
#include "Layer.h"
#include "Building.h"
#include "Entity.h"
#include "CameraManager.h"

class BuildingManager
{
private:
    Vector3 defaultBuildingSize;
    Color defaultBuildingColor;
    Layer* layer;

    std::vector<Building*> buildings;
    std::deque<Building*> buildQueue;
    Building* ghostBuilding;
    bool ghostBuildingIsColliding;

    std::vector<Entity*> entities;

    void updateGhostBuilding();

    template<typename Container>
    bool isColliding(const Container& buildings, Building* targetBuilding);

public:
    BuildingManager() = delete;
    BuildingManager(Vector3 defaultBuildingSize, Color defaultBuildingColor, Layer* layer);
    ~BuildingManager();

    void draw();
    void update();

    std::vector<Building*> getBuildings(); // TODO: prob remove this
    Building* raycastToBuilding();
    void removeBuilding(Building* building);

    Building* yieldBuildQueue();
    Building* buildQueueFront();
    void clearBuildQueue();

    void createNewGhostBuilding(BuildingType buildingType);
    void clearGhostBuilding();
    void scheduleGhostBuilding();
    bool canScheduleGhostBuilding();
    Building* getGhostBuilding();
    bool ghostBuildingExists();

    std::vector<Entity*> getEntities();
};

#endif
