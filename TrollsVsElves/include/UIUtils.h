#pragma once

#include "rlImGui.h"
#include "imgui.h"

inline int pushButtonDisabled() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.83f, 0.32f, 0.32f, 0.4f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.83f, 0.32f, 0.32f, 0.7f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.83f, 0.32f, 0.32f, 1.00f});
    return 3;
}

inline int pushButtonEnabled() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.26f, 0.59f, 0.98f, 0.40f});         // found in imgui_draw.cpp@201
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.26f, 0.59f, 0.98f, 1.00f});  // found in imgui_draw.cpp@202
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.06f, 0.53f, 0.98f, 1.00f});   // found in imgui_draw.cpp@203
    return 3;
}