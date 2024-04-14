#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "BuildingManager.h"
#include "AdvancementTree.h"

#include <map>
#include <functional>

class Player : public Entity
{
private:
    BuildingManager* buildingManager;

    AdvancementTree* advancements;
    AdvancementNode* currentAdvancements;

    void checkKeyboardPresses(std::vector<AdvancementNode*> children);
    void handleButtonPressLogic(AdvancementNode* node);

public:
    Player() = delete;
    Player(Capsule capsule, Vector3 speed, BuildingManager* buildingManager);
    ~Player();

    void draw();
    void drawUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine);
    void update();

    void deselect() override;
};

#endif
