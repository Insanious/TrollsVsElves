#include "Building.h"

Building::Building(
    Cube cube,
    BuildingType buildingType,
    AdvancementNode* advancement,
    simpleSignal::Signal<void(std::string)>* promotionSignal)
{

    buildStage = GHOST;

    ghostColor = { 0, 121, 241, 100 };
    inProgressColor = { 255, 255, 255, 100 };

    this->promotionSignal = promotionSignal;
    this->cube = cube;
    this->buildingType = buildingType;
    cube.color = ghostColor;

    selected = false;
    sold = false;
    recruiting = false;

    buildTime = 0.2f;
    buildTimer = 0.f;

    rallyPoint = Cylinder(Vector3Zero(), 0.8f, 20.f, 8, { 255, 255, 255, 128 });

    this->advancement = advancement;

    switch (buildingType)
    {
        case ROCK:
            targetColor = Color{ 60, 60, 60, 255 };
            canRecruit = false;
            break;
        case CASTLE:
            targetColor = BEIGE;
            canRecruit = true;
            break;
    }

    Vector3 targetColorHSL = ColorToHSV(targetColor);
    selectedColor = ColorFromHSV(targetColorHSL.x, targetColorHSL.y, targetColorHSL.z - 0.2f);
}

Building::~Building() {}

void Building::draw()
{
    drawCube(cube);

    if (selected && !Vector3Equals(rallyPoint.position, cube.position)) // This might be a temporary solution TODO: later
        drawCylinder(rallyPoint);
}

void Building::drawUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine)
{
    std::vector<AdvancementNode*> children = advancement->children;

    AdvancementNode fillerButton = AdvancementNode(nullptr, "filler", "filler", {});
    AdvancementNode sellButton = AdvancementNode(nullptr, "sell", "Sell", {});

    int nrOfFillerButtons = nrOfButtons - children.size();
    for (int i = 0; i < nrOfFillerButtons - 1; i++)
        children.push_back(&fillerButton);  // add filler buttons between actual buttons and sell button
    children.push_back(&sellButton);        // add sellButton last so its the last button

    AdvancementNode* child = nullptr;
    bool buttonWasPressed = false;
    for (int i = 0; i < children.size(); i += buttonsPerLine)
    {
        for (int j = 0; j < buttonsPerLine; j++)
        {
            child = children[i+j];

            if (child->name == "filler")
                ImGui::InvisibleButton(child->name.c_str(), buttonSize);
            else
            {
                if (canBePromotedTo(child)) // push default colors
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.26f, 0.59f, 0.98f, 0.40f});         // found in imgui_draw.cpp@201
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.26f, 0.59f, 0.98f, 1.00f});  // found in imgui_draw.cpp@202
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.06f, 0.53f, 0.98f, 1.00f});   // found in imgui_draw.cpp@203
                }
                else // push disabled colors
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.83f, 0.32f, 0.32f, 0.4f});
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.83f, 0.32f, 0.32f, 0.7f});
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.83f, 0.32f, 0.32f, 1.00f});
                }

                if (ImGui::Button(child->name.c_str(), buttonSize))
                {
                    buttonWasPressed = true;
                    if (child->id == "sell")
                        sell();
                    else if (canBePromotedTo(child))
                        promote(child);
                }

                ImGui::PopStyleColor(3); // remove pushed colors
            }

            if (j != buttonsPerLine - 1) // apply on all except the last
                ImGui::SameLine();
        }
    }

    if (!buttonWasPressed)
        checkKeyboardPresses(children);
}

void Building::checkKeyboardPresses(std::vector<AdvancementNode*> children)
{
    // check if any key between KEY_ONE -> KEY_ONE + children.size() was pressed
    AdvancementNode* child = nullptr;
    for (int i = 0; i < children.size(); i++)
    {
        child = children[i];
        int keyNr = int(KEY_ONE) + i;
        if (IsKeyPressed((KeyboardKey)keyNr))
        {
            if (child->name == "filler")
                return;
            else if (child->id == "sell")
                sell();
            else if (canBePromotedTo(child))
                promote(child);
            break;
        }
    }
}

void Building::update()
{
    if (buildStage == IN_PROGRESS)
    {
        buildTimer += GetFrameTime();
        float adjusted = buildTimer/buildTime;
        cube.color = lerpColor(inProgressColor, targetColor, adjusted);

        if (buildTimer >= buildTime)
        {
            cube.color = selected ? selectedColor : targetColor;
            buildTimer = 0.f;
            buildStage = FINISHED;
        }
    }
}

void Building::scheduleBuild()
{
    assert(buildStage == GHOST); // sanity check

    buildStage = SCHEDULED;
    cube.color = inProgressColor;
    rallyPoint.position = { cube.position.x, cube.position.y, cube.position.z };
}

void Building::build()
{
    assert(buildStage == SCHEDULED); // sanity check

    promotionSignal->emit(advancement->id);
    buildStage = IN_PROGRESS;
}

void Building::setPosition(Vector3 position)
{
    cube.position = position;
}

Vector3 Building::getPosition()
{
    return cube.position;
}

Cube& Building::getCube()
{
    return cube;
}

Color Building::getGhostColor()
{
    return ghostColor;
}

void Building::select()
{
    selected = true;
    if (buildStage != IN_PROGRESS)
        cube.color = selectedColor;
}

void Building::deselect()
{
    selected = false;
    if (buildStage != IN_PROGRESS)
        cube.color = targetColor;
}

bool Building::isSelected()
{
    return selected;
}

void Building::sell()
{
    sold = true;
}

bool Building::isSold()
{
    return sold;
}

bool Building::isRecruiting()
{
    return recruiting;
}

void Building::setRecruiting(bool value)
{
    recruiting = value;
}

Cylinder Building::getRallyPoint()
{
    return rallyPoint;
}

void Building::setRallyPoint(Vector3 point)
{
    rallyPoint.position = point;
}

std::vector<AdvancementNode*> Building::getPossiblePromotions()
{
    return advancement->children;
}

bool Building::canBePromotedTo(AdvancementNode* promotion)
{
    return std::find(lockedPromotions.begin(), lockedPromotions.end(), promotion->id) == lockedPromotions.end();
}

void Building::promote(AdvancementNode* promotion)
{
    printf("'%s' got promoted to '%s'\n", advancement->id.c_str(), promotion->id.c_str());
    advancement = promotion;
    promotionSignal->emit(promotion->id);
}

void Building::updateLockedPromotions(std::vector<std::string> lockedPromotions)
{
    this->lockedPromotions = lockedPromotions;
}