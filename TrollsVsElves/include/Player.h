#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "BuildingManager.h"
#include "AdvancementTree.h"

#include <map>
#include <functional>

enum PlayerType { PLAYER_ELF, PLAYER_TROLL };

class Player : public Entity
{
private:
    BuildingManager* buildingManager;
    PlayerType type;

    AdvancementTree* advancements;
    AdvancementNode* currentAdvancements;

    void checkKeyboardPresses(std::vector<AdvancementNode*> children);
    void handleButtonPressLogic(AdvancementNode* node);

public:
    Player() = delete;
    Player(Vector3 position, Vector3 speed, BuildingManager* buildingManager, PlayerType type);
    ~Player();

    bool isElf();
    bool isTroll();

    void draw();
    void drawUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine);
    void update();

    void deselect() override;
};

#endif
