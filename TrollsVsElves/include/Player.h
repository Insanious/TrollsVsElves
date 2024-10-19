#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "ActionsManager.h"

#include "NetworkIDObject.h"

#include <map>
#include <functional>

class BuildingManager; // forward declaration to get around circular depenedency

enum PlayerType { PLAYER_ELF, PLAYER_TROLL };

struct Player : public Entity, public RakNet::NetworkIDObject
{
    BuildingManager* buildingManager;

    std::string actionId;
    std::string originalActionId;
    std::string previousActionId;

    PlayerType playerType;

    Player() = delete;
    Player(Vector3 position, PlayerType playerType);
    ~Player();

    void draw();
    std::vector<ActionNode> getActions(int nrOfButtons);
    void update();

    void deselect() override;
};

#endif
