#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "structs.h"

#include "rcamera.h"

#include "BaseScreen.h"
#include "Layer.h"
#include "Building.h"
#include "Player.h"
#include <vector>
#include <deque>
#include <map>

struct UIMapping
{
    KeyboardKey key = KEY_NULL;
    std::string buttonText = "";

    UIMapping(): key(), buttonText() {};

    UIMapping(KeyboardKey _key, std::string _buttonText)
        : key(_key), buttonText(_buttonText) {};
};

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

        std::map<BUILDING_TYPE, UIMapping> buildingTypeMappings;

        bool hoveringUI;

        Player* player;
        bool showPlayer;

        Camera3D camera;
        bool isGhostBuildingColliding = false;

    public:
        GameScreen();
        ~GameScreen();

        void init(Vector2i screenSize);
        void draw();
        void drawUI();
        void update();
        void updateCamera();
        void updateGhostBuilding();
        void updateSelectedBuilding();
        void updateBuildQueue();
        void ghostBuildingCollision();

        void handleLeftMouseButton();
        void handleRightMouseButton();

        void createNewGhostBuilding(BUILDING_TYPE buildingType);

        Building* raycastToBuilding();
        RayCollision raycastToGround();

        Vector3 calculateTargetPositionToBuildingFromPlayer(Building* building);
};

#endif
