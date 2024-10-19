#ifndef BUILDING_MANAGER_H
#define BUILDING_MANAGER_H

#include <vector>
#include <deque>
#include <unordered_map>
#include <optional>

#include "utils.h"
#include "MapGenerator.h"
#include "CameraManager.h"
#include "ActionsManager.h"
#include "UIUtils.h"

class Player; // forward declaration to get around circular depenedency

enum BuildStage { GHOST = 0, SCHEDULED, IN_PROGRESS, FINISHED };
enum BuildingType { CASTLE = 0, ROCK, HALL, SHOP };

struct Building
{
    Cube cube;
    BuildStage buildStage = GHOST;

    Color ghostColor = { 0, 121, 241, 100 };
    Color inProgressColor = { 255, 255, 255, 100 };
    Color selectedColor;
    Color targetColor;

    float buildTime = 0.2f;
    float buildTimer = 0.f;

    bool selected = false;

    Cylinder rallyPoint = Cylinder(Vector3Zero(), 0.8f, 20.f, 8, { 255, 255, 255, 128 });

    Player* owner;
    BuildingType buildingType;
    std::string actionId;
    std::vector<std::string> previousActionIds;
    bool sold = false;

    Building() = delete;
    Building(Cube _cube, BuildingType _buildingType, Player* _owner) : cube(_cube), buildingType(_buildingType), owner(_owner)
    {
        switch (buildingType)
        {
            case ROCK:      targetColor = Color{ 100, 100, 100, 255 };  actionId = "rock0";     break;
            case CASTLE:    targetColor = BEIGE;                        actionId = "castle0";   break;
            case HALL:      targetColor = BLUE;                         actionId = "hall0";     break;
            case SHOP:      targetColor = SKYBLUE;                      actionId = "shop0";     break;
        }

        Vector3 targetColorHSL = ColorToHSV(targetColor);
        selectedColor = ColorFromHSV(targetColorHSL.x, targetColorHSL.y, targetColorHSL.z * 0.8f);
    }

    ~Building() {}
};

struct OptionalBuilding
{
    std::optional<Building> _building;
    bool isColliding;

    void set(Building building) { _building = building; }
    void reset()                { _building.reset(); }
    bool exists()               { return _building.has_value(); }
    Building& get()             { return _building.value(); }
};

struct BuildingManager
{
    Vector3 defaultBuildingSize;
    Color defaultBuildingColor;

    std::vector<Building> buildings;
    std::deque<Building> buildQueue;
    OptionalBuilding ghost;
    int selectedIndex;

    MapGenerator* mapGenerator;

    std::unordered_map<std::string, unsigned> unlockedActions;

    void updateGhostBuilding();

    template<typename Container>
    bool isColliding(const Container& buildings, Building* targetBuilding);

    BuildingManager() = delete;
    BuildingManager(Vector3 defaultBuildingSize, Color defaultBuildingColor, MapGenerator* mapGenerator);
    ~BuildingManager();

    void draw();
    void drawBuildingUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine);
    void resolveBuildingAction(Building& building, ActionNode& action);
    void update();

    Building* raycastToBuilding();
    void removeBuilding(size_t index);

    Building* yieldBuildQueue();
    Building* buildQueueFront();
    void clearBuildQueue();

    void createDebugBuilding(Vector2i index, BuildingType type);
    void createNewGhostBuilding(BuildingType buildingType, Player* player);
    void scheduleGhostBuilding();
    void progressBuilding(Building& building, BuildStage stage);

    void select(Building* building);
    void deselect();

    void recruit(Building* building);
    bool canPromoteTo(std::string id);
    void promote(Building& building, std::string id);
};

#endif
