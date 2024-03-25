#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "globals.h"

#include "rcamera.h"

#include "BaseScreen.h"
#include "Layer.h"
#include "Building.h"
#include "BuildingUI.h"
#include <vector>

class GameScreen: public BaseScreen
{
    private:
        Vector2i screenSize;

        std::vector<Layer*> layers;
        Vector2i gridSize;
        Vector3 cubeSize;
        Color defaultCubeColor;

        std::vector<Building*> buildings;
        Color defaultBuildingColor;
        Vector3 buildingSize;

        Building* floatingBuilding;
        Building* selectedBuilding;

        BuildingUI buildingUI;

        Camera3D camera;

    public:
        GameScreen();
        ~GameScreen();

        void init(Vector2i screenSize);
        void draw();
        void update();
        void updateCamera();
        void updateFloatingBuilding();
        void updateSelectedBuilding();

        Building* raycastToNearestBuilding();
};

#endif
