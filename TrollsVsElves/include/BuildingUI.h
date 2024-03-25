#ifndef BUILDING_U_I_H
#define BUILDING_U_I_H

#include "imgui.h"
#include "rlImGui.h"
#include "globals.h"
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

    void init(Building* building);
    void update();
    void draw();

    void setScreenSize(Vector2i size);

    void show();
    void hide();
    bool isHovering();

};

#endif
