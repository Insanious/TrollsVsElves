#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include "BuildingManager.h"
#include "Player.h"
#include "utils.h"

class PlayerManager
{
private:
    BuildingManager* buildingManager;

public:
    std::vector<Player*> players;
    Player* selectedPlayer;

    PlayerManager() = delete;
    PlayerManager(BuildingManager* buildingManager);
    ~PlayerManager();

    void draw();
    void update();
    void addPlayer(Player* player);
    void select(Player* player);
    void deselect();

    Vector3 calculateTargetPositionToCubeFromPlayer(Player* player, Cube cube);
    bool checkCollisionCapsulePoint(Capsule capsule, Vector2 point);

    void pathfindPlayerToCube(Player* player, Cube cube);

    Player* raycastToPlayer();
};

#endif
