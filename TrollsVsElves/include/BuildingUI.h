#ifndef BUILDING_U_I_H
#define BUILDING_U_I_H

#include "imgui.h"
#include "rlImGui.h"
#include "utils.h"
#include "Building.h"

class BuildingUI
{
private:
    Vector2i screenSize;

    Building* building;

    bool visible;
    bool hovering;

public:
    BuildingUI();
    ~BuildingUI();

    void update();
    void draw();

    void setScreenSize(Vector2i size);

    void showBuilding(Building* building);
    void hide();
    bool isHovering();

};

#endif
