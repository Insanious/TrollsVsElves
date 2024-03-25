#include "BuildingUI.h"

BuildingUI::BuildingUI()
{
    screenSize = { 0, 0 };
    building = nullptr;
    visible = false;
}

BuildingUI::~BuildingUI()
{
}

void BuildingUI::init(Building* building)
{
    this->building = building;
    printf("BuildingUI::init\n");
}

void BuildingUI::update()
{
    printf("BuildingUI::update\n");
}

void BuildingUI::draw()
{

    if (visible)
    {
        bool open;

        int flags = 0;
        flags |= ImGuiWindowFlags_NoTitleBar;
        flags |= ImGuiWindowFlags_NoResize;
        flags |= ImGuiWindowFlags_NoMove;
        // flags |= ImGuiWindowFlags_NoScrollWithMouse;

        ImVec2 windowSize(200, 100);
        ImVec2 windowPos(this->screenSize.x - windowSize.x, this->screenSize.y - windowSize.y);
        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(windowPos);

        ImVec2 windowPadding(8, 8);
        ImVec2 buttonPadding(8, 8);
        ImVec2 buttonSize(windowSize.x / 2 - windowPadding.x - buttonPadding.x / 2, windowSize.y - windowPadding.y * 2);


        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);
        ImGui::Begin("BuildingUI window", &open, flags);
        ImGui::PopStyleVar();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, buttonPadding);
        // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        if (ImGui::Button("Upgrade", buttonSize))
            building->upgrade();
        ImGui::SameLine();
        if (ImGui::Button("Sell", buttonSize))
            building->sell();
        // ImGui::Button("Upgrade3", buttonSize);
        // ImGui::Button("Upgrade4", buttonSize);
        // ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        this->hovering = ImGui::IsMouseHoveringRect(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y));

        // ImGui::NewFrame();

        // // Draw text at a specific position
        // ImVec2 textPosition(100, 100);
        // ImGui::SetCursorScreenPos(textPosition);
        // ImGui::Text("Hello, ImGui!");

        ImGui::End();

        bool openUpgrade = true;
        int upgradeFlags = 0;
        upgradeFlags |= ImGuiWindowFlags_NoInputs;
        upgradeFlags |= ImGuiWindowFlags_NoTitleBar;
        upgradeFlags |= ImGuiWindowFlags_NoResize;
        upgradeFlags |= ImGuiWindowFlags_NoMove;
        ImGui::SetNextWindowSize(ImVec2(100, 20));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("BuildingUI upgrade level window", &openUpgrade, upgradeFlags);
        // ImGui::SetCursorScreenPos(ImVec2(100, 100));
        // ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.z));
        ImGui::Text("Level: %d", building->getLevel());
        ImGui::End();

    }
}

void BuildingUI::setScreenSize(Vector2i size)
{
    screenSize = size;
}

void BuildingUI::show()
{
    visible = true;
    printf("BuildingUI::show\n");
}

void BuildingUI::hide()
{
    visible = false;
    printf("BuildingUI::hide\n");
}

bool BuildingUI::isHovering()
{
    if (!visible) return false;

    return hovering;
}
