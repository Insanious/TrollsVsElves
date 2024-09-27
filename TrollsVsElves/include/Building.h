#ifndef BUILDING_H
#define BUILDING_H

#include "structs.h"
#include "imgui.h"
#include "rlImGui.h"

enum BuildStage { GHOST = 0, SCHEDULED, IN_PROGRESS, FINISHED };
enum BuildingType { CASTLE = 0, ROCK, HALL, SHOP };

class Building
{
private:
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

public:
    BuildingType buildingType;
    std::string actionId;
    std::vector<std::string> previousActionIds;
    bool sold;

    Building() = delete;
    Building(Cube cube, BuildingType buildingType);
    ~Building();

    void draw();
    void update();

    void scheduleBuild();
    void build();

    void setPosition(Vector3 position);
    Vector3 getPosition();
    Cube& getCube();
    Color getGhostColor();

    void select();
    void deselect();
    bool isSelected();

    Cylinder getRallyPoint();
    void setRallyPoint(Vector3 point);
};

#endif
