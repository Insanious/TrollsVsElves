#include "Player.h"

Player::Player(Capsule capsule, Vector3 speed, BuildingManager* buildingManager)
    : Entity(capsule, speed, PLAYER)
{
    this->buildingManager = buildingManager;

    advancements = new AdvancementTree("player-dependencies.json");
    currentAdvancements = advancements->getRoot();
}

Player::~Player() {}

void Player::draw()
{
    Entity::draw();
}

void Player::drawUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine)
{
    std::vector<AdvancementNode*> children = currentAdvancements->children;

    AdvancementNode fillerButton = AdvancementNode(nullptr, "filler", "filler", {});
    AdvancementNode backButton = AdvancementNode(nullptr, "back", "Back", {});

    int nrOfFillerButtons = nrOfButtons - children.size();
    for (int i = 0; i < nrOfFillerButtons - 1; i++)
        children.push_back(&fillerButton);          // add filler buttons between actual buttons and back button

    if (nrOfFillerButtons)
    {
        if (currentAdvancements->parent)            // can go back from here
            children.push_back(&backButton);        // add backButton last so its the last button
        else
            children.push_back(&fillerButton);      // can't go back from here so add another fillerButton
    }


    bool buttonWasPressed = false;
    AdvancementNode* child = nullptr;
    for (int i = 0; i < children.size(); i += buttonsPerLine)
    {
        for (int j = 0; j < buttonsPerLine; j++)
        {
            child = children[i+j];

            if (child->name == "filler") // draw invisible button, don't care if its pressed or not
                ImGui::InvisibleButton(child->name.c_str(), buttonSize);
            else if (ImGui::Button(child->name.c_str(), buttonSize))
            {
                buttonWasPressed = true;
                handleButtonPressLogic(child);
            }

            if (j != buttonsPerLine - 1) // apply on all except the last
                ImGui::SameLine();
        }
    }

    if (!buttonWasPressed)
        checkKeyboardPresses(children);
}

void Player::update()
{
    Entity::update();
}

void Player::checkKeyboardPresses(std::vector<AdvancementNode*> children)
{
    // check if any key between KEY_ONE -> KEY_ONE + children.size() was pressed
    AdvancementNode* child = nullptr;
    for (int i = 0; i < children.size(); i++)
    {
        child = children[i];
        int keyNr = int(KEY_ONE) + i;
        if (IsKeyPressed((KeyboardKey)keyNr))
        {
            handleButtonPressLogic(child);
            break;
        }
    }
}

void Player::deselect()
{
    currentAdvancements = advancements->getRoot(); // reset tree to root if player gets deselected
    Entity::deselect();
}

void Player::handleButtonPressLogic(AdvancementNode* node)
{
    if (node->id == "filler")
        return;
    else if (node->id == "back")
        currentAdvancements = currentAdvancements->parent;
    else if (node->id == "castle")
        buildingManager->createNewGhostBuilding(CASTLE);
    else if (node->id == "rock")
        buildingManager->createNewGhostBuilding(ROCK);
    else if (node->id == "hall")
        buildingManager->createNewGhostBuilding(HALL);
    else if (node->id == "blink")
        printf("Blink is not implemented\n"); // TODO: later
    else
        currentAdvancements = node;
}