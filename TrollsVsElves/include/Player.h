#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "ActionsManager.h"
#include "UIUtils.h"

#include "NetworkIDObject.h"

#include <map>
#include <functional>

class BuildingManager; // forward declaration to get around circular depenedency

enum PlayerType { PLAYER_ELF, PLAYER_TROLL };

class Player : public Entity, public RakNet::NetworkIDObject
{
private:
    BuildingManager* buildingManager;

    std::string actionId;
    std::string originalActionId;
    std::string previousActionId;

    std::vector<Item> items;

    void resolveAction(ActionNode& node);

public:
    PlayerType playerType;

    Player() = delete;
    Player(Vector3 position, PlayerType playerType);
    ~Player();

    void setBuildingManager(BuildingManager* buildingManager);
    void tryBuyItem(Item item);

    void draw();
    void drawUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine);
    void update();

    void deselect() override;
};

#endif
