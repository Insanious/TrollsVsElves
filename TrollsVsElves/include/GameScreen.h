#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "structs.h"

#include "rcamera.h"

#include "BaseScreen.h"
#include "Layer.h"
#include "Building.h"
#include "BuildingUI.h"
#include "Player.h"
#include <vector>
#include <deque>

class GameScreen: public BaseScreen
{
    private:
        Vector2i screenSize;

        std::vector<Layer*> layers;
        Vector2i gridSize;
        Vector3 cubeSize;
        Color defaultCubeColor;

        std::vector<Building*> buildings;
        std::deque<Building*> buildQueue;
        Color defaultBuildingColor;
        Vector3 buildingSize;

        Building* ghostBuilding;
        Building* selectedBuilding;

        BuildingUI buildingUI;

        Player* player;

        Camera3D camera;

    public:
        GameScreen();
        ~GameScreen();

        void init(Vector2i screenSize);
        void draw();
        void update();
        void updateCamera();
        void updateGhostBuilding();
        void updateSelectedBuilding();
        void updateBuildQueue();

        Building* raycastToNearestBuilding();
        RayCollision raycastToGround();

        Vector3 calculateTargetPositionToBuildingFromPlayer(Building* building);
};

#endif
