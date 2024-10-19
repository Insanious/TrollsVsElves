#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "structs.h"

#include "BaseScreen.h"
#include "MapGenerator.h"
#include "BuildingManager.h"
#include "PlayerManager.h"
#include "CameraManager.h"
#include "ActionsManager.h"
#include "ThreadSafeMessageQueue.h"

#include <chrono>
#include <vector>
#include <deque>
#include <map>
#include <variant>

enum RaycastHitType {
    RAYCAST_HIT_TYPE_UI = 0,
    RAYCAST_HIT_TYPE_PLAYER,
    RAYCAST_HIT_TYPE_ENTITY,
    RAYCAST_HIT_TYPE_BUILDING,
    RAYCAST_HIT_TYPE_GROUND,
    RAYCAST_HIT_TYPE_OUT_OF_BOUNDS
};

struct RayCollisionObject
{
    RaycastHitType type;
    std::variant<std::monostate, Player*, Entity*, Building*, Cube*> object;
};

struct NetworkManager; // forward declaration to get around circular depenedency

class GameScreen: public BaseScreen
{
    private:
        Vector2i screenSize;

        bool isGhostBuildingColliding = false;

        bool isMultiSelecting;
        Vector2 multiSelectionStartPosition;
        Rectangle multiSelectionRectangle;

        std::chrono::steady_clock::time_point lastLeftMouseButtonClick;

    public:
        BuildingManager* buildingManager;
        PlayerManager* playerManager;
        NetworkManager* networkManager;
        MapGenerator* mapGenerator;
        ThreadSafeMessageQueue messageQueue;

        GameScreen() = delete;
        GameScreen(Vector2i screenSize, bool isSinglePlayer);
        ~GameScreen();

        void draw();
        void drawUI();
        void update();

        void startMultiSelection();
        void stopMultiSelection();
        void updateMultiSelection();

        RayCollisionObject raycastWorld();
        void handleLeftMouseButton();
        void handleRightMouseButton();
};

#endif
