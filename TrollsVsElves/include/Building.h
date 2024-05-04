#ifndef BUILDING_H
#define BUILDING_H

#include "structs.h"
#include "imgui.h"
#include "rlImGui.h"
#include "AdvancementTree.h"

enum BuildStage { GHOST = 0, SCHEDULED, IN_PROGRESS, FINISHED };
enum BuildingType { CASTLE = 0, ROCK, HALL, SHOP };

class Building
{
private:
    Cube cube;
    BuildStage buildStage;
    BuildingType buildingType;

    Color ghostColor;
    Color inProgressColor;
    Color selectedColor;
    Color targetColor;

    float buildTime;
    float buildTimer;

    bool selected;
    bool sold;

    Cylinder rallyPoint;

    AdvancementNode* advancement;

public:
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

    void sell();
    bool isSold();

    Cylinder getRallyPoint();
    void setRallyPoint(Vector3 point);

    AdvancementNode* getAdvancement();
    void promote(AdvancementNode* promotion);
};

#endif
