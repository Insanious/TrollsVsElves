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
    Player* clientPlayer;

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
    std::vector<Vector3> pathfindPlayerToPosition(Player* player, Vector3 position);

    Player* raycastToPlayer();
    Player* getPlayerWithNetworkID(RakNet::NetworkID networkID);
};

#endif
