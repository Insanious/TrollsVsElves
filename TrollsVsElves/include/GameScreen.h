#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "globals.h"

#include "rcamera.h"

#include "BaseScreen.h"
#include "Layer.h"
#include "Building.h"
#include <vector>

class GameScreen: public BaseScreen
{
    private:
        Vector2i gridSize;
        Vector3 cubeSize;
        Color defaultCubeColor;
        Color defaultBuildingColor;
        Vector3 buildingSize;

        std::vector<Layer*> layers;
        std::vector<Building*> buildings;

        Building* floatingBuilding;
        Building* selectedBuilding;

        Camera3D camera;

    public:
        GameScreen();
        ~GameScreen();

        void init();
        void draw();
        void update();
        void updateCamera();
        void updateFloatingBuilding();

        Building* raycastToNearestBuilding();
};

#endif
