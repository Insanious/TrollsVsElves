#include "Player.h"
#include "BuildingManager.h"

Player::Player(Vector3 position, PlayerType playerType)
    : Entity(position, BLANK, PLAYER)
{
    this->playerType = playerType;

    switch (playerType)
    {
        case PLAYER_ELF:
            this->capsule = Capsule(2.f, 8.f);
            this->setDefaultColor(BLUE);
            this->originalActionId = "elf";
            break;

        case PLAYER_TROLL:
            this->capsule = Capsule(3.f, 12.f);
            this->setDefaultColor(RED);
            this->originalActionId = "troll";
            break;
    }

    setPosition(position);
    actionId = this->originalActionId;
}

Player::~Player() {}

void Player::draw()
{
    Entity::draw();
}

std::vector<ActionNode> Player::getActions(int nrOfButtons)
{
    std::vector<ActionNode> children = ActionsManager::get().getActionChildren(actionId);

    ActionNode fillerButton = ActionNode("filler", "Filler", "filler", {});
    ActionNode backButton = ActionNode("back", "Back", "back", {});

    int nrOfFillerButtons = nrOfButtons - children.size();
    for (int i = 0; i < nrOfFillerButtons - 1; i++)
        children.push_back(fillerButton);           // add filler buttons between actual buttons and back button
    if (nrOfFillerButtons)                          // add back button if possible
        children.push_back(actionId != originalActionId ? backButton : fillerButton);

    for (ActionNode& node: children)
    {
        node.promotable = true;
        if (node.promotable)
        {
            if (node.id == "filler")
                node.callback = []() {};
            else if (node.action == "back")
                node.callback = [this]() { this->actionId = this->previousActionId; };
            else if (node.action == "build")
            {
                if (node.id == "castle0")       node.callback = [this] { this->buildingManager->createNewGhostBuilding(CASTLE, this); };
                else if (node.id == "rock0")    node.callback = [this] { this->buildingManager->createNewGhostBuilding(ROCK, this); };
                else if (node.id == "hall0")    node.callback = [this] { this->buildingManager->createNewGhostBuilding(HALL, this); };
                else if (node.id == "shop0")    node.callback = [this] { this->buildingManager->createNewGhostBuilding(SHOP, this); };
            }
            else if (node.id == "blink")
                node.callback = []() { printf("Blink is not implemented\n"); };
            else if (node.action == "promote")
                node.callback = [this, node]() {
                    this->previousActionId = this->actionId;
                    this->actionId = node.id;
                };
        }
    }

    return children;
}

void Player::update()
{
    Entity::update();
}

void Player::deselect()
{
    actionId = originalActionId;
    Entity::deselect();
}
