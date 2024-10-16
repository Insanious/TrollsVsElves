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

void Player::drawUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine)
{
    std::vector<ActionNode> children = ActionsManager::get().getActionChildren(actionId);

    ActionNode fillerButton = ActionNode("filler", "Filler", "filler", {});
    ActionNode backButton = ActionNode("back", "Back", "back", {});

    int nrOfFillerButtons = nrOfButtons - children.size();
    for (int i = 0; i < nrOfFillerButtons - 1; i++)
        children.push_back(fillerButton);           // add filler buttons between actual buttons and back button
    if (nrOfFillerButtons)                          // add back button if possible
        children.push_back(actionId != originalActionId ? backButton : fillerButton);

    bool buttonWasPressed = false;
    ActionNode child;
    for (int i = 0; i < children.size(); i += buttonsPerLine)
    {
        for (int j = 0; j < buttonsPerLine; j++)
        {
            child = children[i+j];

            if (child.id == "filler") // draw invisible button, don't care if its pressed or not
                ImGui::InvisibleButton(child.name.c_str(), buttonSize);
            else
            {
                int colors = pushButtonEnabled();
                if (ImGui::Button(child.name.c_str(), buttonSize))
                {
                    buttonWasPressed = true;
                    resolveAction(child);
                }

                ImGui::PopStyleColor(colors);
            }

            if (j != buttonsPerLine - 1) // apply on all except the last
                ImGui::SameLine();
        }
    }

    if (!buttonWasPressed) // check if any button was clicked using number-key buttons
        for (int i = 0; i < children.size(); i++)
            if (IsKeyPressed((KeyboardKey)int(KEY_ONE) + i))
            {
                resolveAction(children[i]);
                break;
            }
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

void Player::resolveAction(ActionNode& node)
{
    if (node.id == "filler")
        return;
    else if (node.action == "back")
        actionId = previousActionId;
    else if (node.action == "build") {
        if (node.id == "castle0")    buildingManager->createNewGhostBuilding(CASTLE, this);
        else if (node.id == "rock0") buildingManager->createNewGhostBuilding(ROCK, this);
        else if (node.id == "hall0") buildingManager->createNewGhostBuilding(HALL, this);
        else if (node.id == "shop0") buildingManager->createNewGhostBuilding(SHOP, this);
    }
    else if (node.id == "blink")
        printf("Blink is not implemented\n"); // TODO: later
    else if (node.action == "promote") {
        previousActionId = actionId;
        actionId = node.id;
    }
}