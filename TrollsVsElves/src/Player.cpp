#include "Player.h"
#include "BuildingManager.h"

Player::Player(Vector3 position, Vector3 speed, PlayerType type)
    : Entity(position, speed, BLANK, PLAYER)
{
    this->type = type;
    switch (type)
    {
        case PLAYER_ELF:
            this->setCapsule(Capsule(2.f, 8.f));
            this->setDefaultColor(BLUE);
            this->setPosition(position);
            advancements = new AdvancementTree("player-dependencies.json");
            break;

        case PLAYER_TROLL:
            this->setCapsule(Capsule(3.f, 12.f));
            this->setDefaultColor(RED);
            this->setPosition(position);
            advancements = new AdvancementTree("troll-dependencies.json"); // TODO: later, create a proper troll-dependencies file
            break;
    }

    currentAdvancements = advancements->getRoot();
}

Player::~Player() {}

void Player::setBuildingManager(BuildingManager* buildingManager)
{
    this->buildingManager = buildingManager;
}

void Player::tryBuyItem(Item item)
{
    bool canBuyItem = this->isTroll();

    printf("%s item called '%s'\n", canBuyItem ? "Purchased" : "Couldn't purchase", item.name.c_str());
    if (!canBuyItem)
        return;

    items.push_back(item);
    if (item.name == "Cocaine")
        setSpeed(Vector3Add(getSpeed(), { 10.f, 10.f, 10.f }));
}

bool Player::isElf()
{
    return type == PLAYER_ELF;
}

bool Player::isTroll()
{
    return type == PLAYER_TROLL;
}

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

    if (nrOfFillerButtons)                          // add back button if possible
        children.push_back(currentAdvancements->parent ? &backButton : &fillerButton);


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