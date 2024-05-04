#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "AdvancementTree.h"

#include <map>
#include <functional>

class BuildingManager; // forward declaration to get around circular depenedency

enum PlayerType { PLAYER_ELF, PLAYER_TROLL };

class Player : public Entity
{
private:
    BuildingManager* buildingManager;
    PlayerType type;

    AdvancementTree* advancements;
    AdvancementNode* currentAdvancements;

    std::vector<Item> items;

    void checkKeyboardPresses(std::vector<AdvancementNode*> children);
    void handleButtonPressLogic(AdvancementNode* node);

public:
    Player() = delete;
    Player(Vector3 position, Vector3 speed, PlayerType type);
    ~Player();

    void setBuildingManager(BuildingManager* buildingManager);
    void tryBuyItem(Item item);

    bool isElf();
    bool isTroll();

    void draw();
    void drawUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine);
    void update();

    void deselect() override;
};

#endif
