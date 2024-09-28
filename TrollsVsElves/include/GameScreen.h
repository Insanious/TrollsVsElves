#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "structs.h"

#include "BaseScreen.h"
#include "MapGenerator.h"
#include "Building.h"
#include "BuildingManager.h"
#include "Player.h"
#include "CameraManager.h"
#include "Resource.h"
#include "ActionsManager.h"

#include <chrono>
#include <vector>
#include <deque>
#include <map>

enum RaycastHitType {
    RAYCAST_HIT_TYPE_UI = 0,
    RAYCAST_HIT_TYPE_PLAYER,
    RAYCAST_HIT_TYPE_ENTITY,
    RAYCAST_HIT_TYPE_BUILDING,
    RAYCAST_HIT_TYPE_RESOURCE,
    RAYCAST_HIT_TYPE_GROUND,
    RAYCAST_HIT_TYPE_OUT_OF_BOUNDS
};

class GameScreen: public BaseScreen
{
    private:
        Vector2i screenSize;

        BuildingManager* buildingManager;
        std::vector<Entity*> selectedEntities;

        std::vector<Resource*> resources;

        Player* player;

        bool isGhostBuildingColliding = false;

        bool isMultiSelecting;
        Vector2 multiSelectionStartPosition;
        Rectangle multiSelectionRectangle;

        std::chrono::steady_clock::time_point lastLeftMouseButtonClick;

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

        void startMultiSelection();
        void stopMultiSelection();
        void updateMultiSelection();

        RaycastHitType checkRaycastHitType();
        void handleLeftMouseButton();
        void handleRightMouseButton();
        bool raycastToPlayer();
        Entity* raycastToEntity();
        Resource* raycastToResource();
        RayCollision raycastToGround();

        void clearAndDeselectAllSelectedEntities();

        Vector3 calculateTargetPositionToCubeFromEntity(Entity* entity, Cube cube);

        void addResource(Vector3 position);
};

#endif
