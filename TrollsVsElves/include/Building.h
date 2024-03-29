#ifndef BUILDING_H
#define BUILDING_H

#include "structs.h"
#include "imgui.h"
#include "rlImGui.h"
#include <cassert>

enum BUILD_STAGE { GHOST = 0, SCHEDULED, IN_PROGRESS, FINISHED };
enum BUILDING_TYPE { NONE = 0, CASTLE, ROCK, COUNT };

class Building
{
private:
    Cube cube;
    BUILD_STAGE buildStage;
    BUILDING_TYPE buildingType;

    Color ghostColor;
    Color inProgressColor;
    Color selectedColor;
    Color targetColor;

    float buildTime;
    float buildTimer;

    bool selected;
    bool sold;
    int level;

public:
    Building();
    ~Building();

    void init(Cube cube, BUILDING_TYPE buildingType);
    void draw();
    void drawUIButtons(ImVec2 windowPadding, ImVec2 buttonSize);
    void update();

    void scheduleBuild();
    void build();

    void setPosition(Vector3 position);
    Vector3 getPosition();
    Cube& getCube();
    Color getGhostColor();

    void select();
    void deselect();

    void sell();
    bool isSold();
    void upgrade();

    int getLevel();
    BUILD_STAGE getBuildStage();

};

#endif
