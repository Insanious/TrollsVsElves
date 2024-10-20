#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include "BuildingManager.h"
#include "Player.h"
#include "utils.h"
#include "constants.h"

struct PlayerManager
{
    BuildingManager* buildingManager;
    MapGenerator* mapGenerator;

    std::vector<Player*> players;
    Player* selectedPlayer;
    Player* clientPlayer;

    PlayerManager() = delete;
    PlayerManager(BuildingManager* buildingManager, MapGenerator* mapGenerator);
    ~PlayerManager();

    void draw();
    void update();
    void addPlayer(Player* player);
    void select(Player* player);
    void deselect();

    Vector3 calculateTargetPositionToCubeFromPlayer(Player* player, Cube cube);
    bool checkCollisionCapsulePoint(Capsule capsule, Vector2 point);

    std::vector<Vector3> pathfindPlayerToPosition(Player* player, Vector3 position);

    Player* raycastToPlayer();
    Player* getPlayerWithNetworkID(RakNet::NetworkID networkID);
};

#endif
