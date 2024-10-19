#pragma once

#include "ActionsManager.h"
#include "rlImGui.h"
#include "imgui.h"

namespace UIManager
{
    int pushButtonDisabled();
    int pushButtonEnabled();
    void drawActionButtons(std::vector<ActionNode>& actions, Vector2i screenSize);
};
