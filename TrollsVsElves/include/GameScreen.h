#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "structs.h"

#include "rcamera.h"

#include "BaseScreen.h"
#include "Layer.h"
#include "Building.h"
#include "BuildingManager.h"
#include "Player.h"
#include "PathFinding.h"
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

        Layer* layer;
        Vector2i gridSize;
        Vector3 cubeSize;
        Color defaultCubeColor;

        BuildingManager* buildingManager;
        Building* selectedBuilding;

        std::map<BuildingType, UIMapping> buildingTypeMappings;

        bool hoveringUI;

        Player* player;

        Camera3D camera3D;
        Matrix cameraViewMatrix;
        Camera2D camera2D;

        bool isGhostBuildingColliding = false;

        bool isSelecting;
        bool canSelect;
        Vector2 selectionStartPosition;
        Rectangle selectionRectangle;

        bool checkCollisionCapsuleRectangle(Capsule capsule, Rectangle rectangle, Camera3D camera);
        float calculateCircleRadius2D(Vector3 position, float radius, Camera3D camera);

    public:
        GameScreen();
        ~GameScreen();

        void init(Vector2i screenSize);
        void draw();
        void drawUI();
        void update();
        void updateCamera();
        void updateSelectedBuilding();
        void updateSelectionRectangle();

        void handleLeftMouseButton();
        void handleRightMouseButton();
        RayCollision raycastToGround();

        Vector3 calculateTargetPositionToBuildingFromPlayer(Building* building);

        std::vector<Vector3> pathfindPositions(Vector3 start, Vector3 goal);
};

#endif
