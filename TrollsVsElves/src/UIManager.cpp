#include "UIManager.h"

namespace UIManager
{
    int pushButtonDisabled()
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.83f, 0.32f, 0.32f, 0.4f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.83f, 0.32f, 0.32f, 0.7f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.83f, 0.32f, 0.32f, 1.00f});
        return 3;
    }

    int pushButtonEnabled()
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.26f, 0.59f, 0.98f, 0.40f});         // found in imgui_draw.cpp@201
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.26f, 0.59f, 0.98f, 1.00f});  // found in imgui_draw.cpp@202
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.06f, 0.53f, 0.98f, 1.00f});   // found in imgui_draw.cpp@203
        return 3;
    }

    void drawActionButtons(std::vector<ActionNode>& actions, Vector2i screenSize)
    {
        bool bottomRightWindow = true;
        int bottomRightWindowFlags = 0;
        // bottomRightWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        bottomRightWindowFlags |= ImGuiWindowFlags_NoTitleBar;
        bottomRightWindowFlags |= ImGuiWindowFlags_NoResize;
        bottomRightWindowFlags |= ImGuiWindowFlags_NoMove;

        ImVec2 windowSize(300, 200);
        ImVec2 windowPos(
            screenSize.x - windowSize.x,
            screenSize.y - windowSize.y
        );

        ImVec2 windowPadding(8, 8);
        ImVec2 buttonPadding(8, 8);
        Vector2i buttonLayout = { 2, 2 }; // 2 columns, 2 rows
        int nrOfButtons = buttonLayout.x * buttonLayout.y;
        ImVec2 buttonSize(
            (windowSize.x / float(buttonLayout.x)) - windowPadding.x - (buttonPadding.x / float(buttonLayout.x)),
            (windowSize.y / float(buttonLayout.y)) - windowPadding.y - (buttonPadding.y / float(buttonLayout.y))
        );

        // Draw bottom right window
        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(windowPos);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);
        ImGui::Begin("Bottom right window", &bottomRightWindow, bottomRightWindowFlags);
        ImGui::PopStyleVar();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, buttonPadding);

        size_t buttonsPerLine = buttonLayout.x;
        ActionNode node;
        bool buttonWasPressed = false;
        for (int i = 0; i < actions.size(); i += buttonsPerLine)
        {
            for (int j = 0; j < buttonsPerLine; j++)
            {
                node = actions[i+j];

                if (node.id == "filler") ImGui::InvisibleButton(node.name.c_str(), buttonSize);
                else
                {
                    int colors = node.promotable ? pushButtonEnabled() : pushButtonDisabled();
                    if (ImGui::Button(node.name.c_str(), buttonSize))
                    {
                        buttonWasPressed = true;
                        node.callback();
                    }

                    ImGui::PopStyleColor(colors); // remove pushed colors
                }

                if (j != buttonsPerLine - 1) // apply on all except the last
                    ImGui::SameLine();
            }
        }

        if (!buttonWasPressed) // check if any button was clicked using number-key buttons
            for (int i = 0; i < actions.size(); i++)
                if (IsKeyPressed((KeyboardKey)int(KEY_ONE) + i))
                {
                    actions[i].callback();
                    break;
                }

        ImGui::PopStyleVar();
        ImGui::End();
    }
}