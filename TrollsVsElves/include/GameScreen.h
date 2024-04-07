#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "structs.h"

#include "BaseScreen.h"
#include "Layer.h"
#include "Building.h"
#include "BuildingManager.h"
#include "Player.h"
#include "PathFinding.h"
#include "CameraManager.h"
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

enum RaycastHitType {
    RAYCAST_HIT_TYPE_UI = 0,
    RAYCAST_HIT_TYPE_PLAYER,
    RAYCAST_HIT_TYPE_BUILDING,
    RAYCAST_HIT_TYPE_GROUND,
    RAYCAST_HIT_TYPE_OUT_OF_BOUNDS
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

        Player* player;

        bool isGhostBuildingColliding = false;

        bool isMultiSelecting;
        Vector2 multiSelectionStartPosition;
        Rectangle multiSelectionRectangle;

        bool checkCollisionCapsuleRectangle(Capsule capsule, Rectangle rectangle);
        bool checkCollisionCapsulePoint(Capsule capsule, Vector2 point);
        float calculateCircleRadius2D(Vector3 position, float radius);

    public:
        GameScreen() = delete;
        GameScreen(Vector2i screenSize);
        ~GameScreen();

        void draw();
        void drawUI();
        void update();
        void updateMultiSelectionRectangle();

        RaycastHitType checkRaycastHitType();
        void handleLeftMouseButton();
        void handleRightMouseButton();
        bool raycastToPlayer();
        RayCollision raycastToGround();

        Vector3 calculateTargetPositionToBuildingFromPlayer(Building* building);

        std::vector<Vector3> pathfindPositions(Vector3 start, Vector3 goal);
};

#endif
