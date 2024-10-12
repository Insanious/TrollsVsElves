#include "BuildingManager.h"
#include "Player.h"

void building_manager_init(BuildingManager* manager, Vector3 defaultBuildingSize, Color defaultBuildingColor) {
    manager->defaultBuildingSize = defaultBuildingSize;
    manager->defaultBuildingColor = defaultBuildingColor;

    manager->ghostBuilding.buildStage == NONE;
    manager->ghostBuildingIsColliding = false;
    manager->selectedBuildingIndex = -1;

    // Make sure zeroth level buildings are always available to build
    manager->unlockedActions["castle0"] = 1;
    manager->unlockedActions["rock0"] = 1;
    manager->unlockedActions["hall0"] = 1;
}

void building_manager_cleanup(BuildingManager* manager) {
    building_manager_clear_queue(manager);
}

void building_manager_draw(BuildingManager* manager) {
    for (Building& building: manager->buildings)
        building_draw(&building);

    for (Building& building: manager->buildQueue)
        building_draw(&building);

    if (manager->ghostBuilding.buildStage == GHOST) {
        Cube* cube = &manager->ghostBuilding.cube;
        cube->position = Vector3AddValue(cube->position, 0.1f); // prevents z-fighting
        building_draw(&manager->ghostBuilding);
        cube->position = Vector3SubtractValue(cube->position, 0.1f); // revert change
    }
}

void building_manager_draw_ui_buttons(BuildingManager* manager, ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine) {
    if (manager->selectedBuildingIndex == -1)
        return;

    Building& building = manager->buildings[manager->selectedBuildingIndex];
    std::vector<ActionNode> children = ActionsManager::get().getActionChildren(building.actionId);

    ActionNode fillerButton = ActionNode("filler", "Filler", "filler", {});
    ActionNode sellButton = ActionNode("sell", "Sell", "sell", {});

    int nrOfFillerButtons = nrOfButtons - children.size();
    for (int i = 0; i < nrOfFillerButtons - 1; i++)
        children.push_back(fillerButton);
    if (nrOfFillerButtons)
        children.push_back(sellButton);

    ActionNode child;
    bool buttonWasPressed = false;
    for (int i = 0; i < children.size(); i += buttonsPerLine) {
        for (int j = 0; j < buttonsPerLine; j++) {
            child = children[i+j];

            if (child.id == "filler")
                ImGui::InvisibleButton(child.name.c_str(), buttonSize);
            else {
                int colors = building_manager_can_promote_to(manager, child.id.c_str()) ? pushButtonEnabled() : pushButtonDisabled();
                if (ImGui::Button(child.name.c_str(), buttonSize)) {
                    buttonWasPressed = true;
                    building_manager_resolve_action(manager, &building, &child);
                }

                ImGui::PopStyleColor(colors);
            }

            if (j != buttonsPerLine - 1)
                ImGui::SameLine();
        }
    }

    if (!buttonWasPressed)
        for (int i = 0; i < children.size(); i++)
            if (IsKeyPressed((KeyboardKey)int(KEY_ONE) + i)) {
                building_manager_resolve_action(manager, &building, &children[i]);
                break;
            }
}

void building_manager_resolve_action(BuildingManager* manager, Building* building, ActionNode* action) {
    if (action->id == "filler")
        return;
    else if (action->action == "sell")
        building->sold = true;
    else if (action->action == "recruit")
        printf("'recruit' action is not implemented\n");
    else if (action->action == "buy")
        printf("'buy' action is not implemented\n");
    else if (action->action == "promote" && building_manager_can_promote_to(manager, action->id.c_str()))
        building_manager_promote(manager, building, action->id.c_str());
}

void building_manager_update(BuildingManager* manager) {
    for (Building& building: manager->buildings)
        building_update(&building);

    for (Building& building: manager->buildQueue)
        building_update(&building);

    if (manager->ghostBuilding.buildStage == BuildStage::GHOST)
        building_manager_update_ghost_building(manager);

    if (manager->selectedBuildingIndex != -1) {
        Building& building = manager->buildings[manager->selectedBuildingIndex];
        if (building.sold)
            building_manager_remove_selected_building(manager);
    }
}

Building* building_manager_raycast(BuildingManager* manager) {
    Ray ray = CameraManager::get().getMouseRay();
    float closestCollisionDistance = std::numeric_limits<float>::infinity();
    Building* nearestBuilding = nullptr;

    for (Building& building: manager->buildings)
    {
        RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(building.cube));

        if (collision.hit && collision.distance < closestCollisionDistance)
        {
            closestCollisionDistance = collision.distance;
            nearestBuilding = &building;
        }
    }

    return nearestBuilding;
}

void building_manager_remove_selected_building(BuildingManager* manager) {
    size_t selected = manager->selectedBuildingIndex;
    size_t last = manager->buildings.size() - 1;
    Building& selectedBuilding = manager->buildings[selected];
    for (std::string& id: selectedBuilding.previousActionIds)
        manager->unlockedActions[id]--;
    manager->unlockedActions[selectedBuilding.actionId]--;

    MapGenerator::get().removeObstacle(selectedBuilding.cube);
    std::swap(selectedBuilding, manager->buildings[last]);
    manager->buildings.pop_back();
    manager->selectedBuildingIndex = -1;
}

Building* building_manager_yield_queue(BuildingManager* manager) {
    assert(manager->buildQueue.size() != 0);

    Building building = manager->buildQueue.front();
    manager->buildQueue.pop_front();
    building_start_build(&building);
    manager->buildings.push_back(building);

    return &manager->buildings.back();
}

bool building_manager_queue_empty(BuildingManager* manager) {
    return manager->buildQueue.empty();
}

Building* building_manager_queue_front(BuildingManager* manager) {
    if (manager->buildQueue.empty())
        return nullptr;

    return &manager->buildQueue.front();
}

void building_manager_clear_queue(BuildingManager* manager) {
    manager->buildQueue.clear();
}

void building_manager_create_debug_building(BuildingManager* manager, Vector2i index, BuildingType type) {
    Building building;
    building_init(&building, Cube(manager->defaultBuildingSize), type, nullptr);
    building.buildStage = BuildStage::GHOST;

    MapGenerator& mapGenerator = MapGenerator::get();
    Vector3 pos = mapGenerator.indexToWorldPosition(index);
    Vector3 cubeSize = mapGenerator.getCubeSize();
    Vector3 snapped = {
        nearestIncrement(pos.x, cubeSize.x),
        pos.y,
        nearestIncrement(pos.z, cubeSize.z)
    };
    Vector3 offset = {
        (cubeSize.x - manager->defaultBuildingSize.x) / 2.0f,
        0.f,
        (cubeSize.z - manager->defaultBuildingSize.z) / 2.0f,
    };

    building.cube.position = Vector3Add(snapped, offset);
    mapGenerator.addObstacle(building.cube);

    building_schedule_build(&building);
    building_start_build(&building);
    building_finish_build(&building);
    manager->buildings.push_back(building);
}

void building_manager_create_ghost_building(BuildingManager* manager, BuildingType buildingType, Player* player) {
    building_init(&manager->ghostBuilding, Cube(manager->defaultBuildingSize), buildingType, player);
    manager->ghostBuilding.buildStage = BuildStage::GHOST;
    building_manager_update_ghost_building(manager);
}

void building_manager_clear_ghost_building(BuildingManager* manager) {
    manager->ghostBuilding.buildStage = BuildStage::NONE;
}

void building_manager_schedule_ghost_building(BuildingManager* manager) {
    assert(manager->ghostBuilding.buildStage == BuildStage::GHOST);

    manager->unlockedActions[manager->ghostBuilding.actionId]++;
    building_schedule_build(&manager->ghostBuilding);
    manager->buildQueue.push_back(manager->ghostBuilding);
    building_manager_clear_ghost_building(manager);
}

bool building_manager_can_schedule_ghost_building(BuildingManager* manager) { // TODO: unnecessary function
    return !manager->ghostBuildingIsColliding;
}

Building* building_manager_get_ghost_building(BuildingManager* manager) {
    return &manager->ghostBuilding;
}

bool building_manager_ghost_building_exists(BuildingManager* manager) {
    return manager->ghostBuilding.buildStage == GHOST;
}

Building* building_manager_get_selected_building(BuildingManager* manager) {
    if (manager->selectedBuildingIndex == -1)
        return nullptr;

    return &manager->buildings[manager->selectedBuildingIndex];
}

void building_manager_select(BuildingManager* manager, Building* building) {
    for (int i = 0; i < manager->buildings.size(); i++) {
        if (&manager->buildings[i] == building) { // TODO: check if this actually works lol
            manager->selectedBuildingIndex = i;
            building_select(building);
            return;
        }
    }
}

void building_manager_deselect(BuildingManager* manager) {
    if (manager->selectedBuildingIndex == -1)
        return;

    building_deselect(&manager->buildings[manager->selectedBuildingIndex]);
    manager->selectedBuildingIndex = -1;
}

void building_manager_recruit(BuildingManager* manager, Building* building) {
    MapGenerator& mapGenerator = MapGenerator::get();

    std::vector<Vector2i> neighboringIndices = mapGenerator.getNeighboringIndices(building->cube);
    if (neighboringIndices.empty()) {
        printf("Found no neighboring tiles, should probably do something about this later\n");
        return;
    }

    Vector3 neighborPosition = mapGenerator.indexToWorldPosition(neighboringIndices[0]);
    float ground = building->cube.position.y;
    Vector3 pos = { neighborPosition.x, ground, neighborPosition.z };
    Vector3 speed = Vector3Scale(Vector3One(), 30);

    // TEMP: disabled
    // Entity* entity = new Entity(pos, speed, BLACK, WORKER);
    // std::vector<Vector3> positions = mapGenerator.pathfindPositions(entity->getPosition(), building->rallyPoint.position);
    // entity->setPositions(positions);
    // entities.push_back(entity);
}

bool building_manager_can_promote_to(BuildingManager* manager, const char* id) {
    std::vector<std::string> requirements = ActionsManager::get().requirements[id];
    for (const std::string& req : requirements)
        if (manager->unlockedActions[req] == 0)
            return false;

    return true;
}

void building_manager_promote(BuildingManager* manager, Building* building, const char* id) {
    printf("promote %s to %s\n", building->actionId.c_str(), id);

    manager->unlockedActions[id]++;
    building->previousActionIds.push_back(building->actionId);
    building->actionId = id;
}

void building_manager_update_ghost_building(BuildingManager* manager) {
    Cube* cubeHit = MapGenerator::get().raycastToGround();
    if (!cubeHit) return;

    Vector3 cubeSize = MapGenerator::get().getCubeSize();
    float ground = MapGenerator::get().getHeight();
    const float max = 10000.f;
    RayCollision collision = GetRayCollisionQuad(
        CameraManager::get().getMouseRay(),
        { -max, ground + (cubeHit->position.y + cubeSize.y/2), -max },
        { -max, ground + (cubeHit->position.y + cubeSize.y/2),  max },
        {  max, ground + (cubeHit->position.y + cubeSize.y/2),  max },
        {  max, ground + (cubeHit->position.y + cubeSize.y/2), -max }
    );

    assert(collision.hit == true);

    float cubeOffset = (int(manager->defaultBuildingSize.x / cubeSize.x)) % 2 == 0
        ? cubeSize.y / 2
        : cubeSize.y;

    Vector3 snapped = {
        nearestIncrement(collision.point.x + cubeOffset, cubeSize.x),
        cubeHit->position.y + cubeSize.y / 2 + manager->defaultBuildingSize.y / 2,
        nearestIncrement(collision.point.z + cubeOffset, cubeSize.z)
    };

    Vector3 offset = {
        (cubeSize.x - manager->defaultBuildingSize.x) / 2.0f,
        0.f,
        (cubeSize.z - manager->defaultBuildingSize.z) / 2.0f,
    };
    Vector3 final = Vector3Add(snapped, offset);

    manager->ghostBuilding.cube.position = final;

    manager->ghostBuildingIsColliding = building_manager_ghost_building_is_colliding(manager);
    manager->ghostBuilding.cube.color = manager->ghostBuildingIsColliding ? RED : manager->ghostBuilding.ghostColor;
}

bool building_manager_ghost_building_is_colliding(BuildingManager* manager) {
    BoundingBox targetBoundingBox = getCubeBoundingBox(manager->ghostBuilding.cube, 0.8f);

    for (const Building& building : manager->buildings)
        if (CheckCollisionBoxes(targetBoundingBox, getCubeBoundingBox(building.cube)))
            return true;

    for (const Building& building : manager->buildQueue)
        if (CheckCollisionBoxes(targetBoundingBox, getCubeBoundingBox(building.cube)))
            return true;

    return false;
}

// BuildingManager::BuildingManager(Vector3 defaultBuildingSize, Color defaultBuildingColor)
// {
//     this->defaultBuildingSize = defaultBuildingSize;
//     this->defaultBuildingColor = defaultBuildingColor;

//     selectedBuilding = nullptr;
//     ghostBuilding = nullptr;
//     ghostBuildingIsColliding = false;

//     // make sure zeroth level buildings are always available to build
//     unlockedActions["castle0"] = 1;
//     unlockedActions["rock0"] = 1;
//     unlockedActions["hall0"] = 1;
// }

// BuildingManager::~BuildingManager()
// {
//     for (int i = 0; i < buildings.size(); i++)
//         delete buildings[i];

//     clearBuildQueue();

//     if (ghostBuilding)
//         delete ghostBuilding;
// }

// void BuildingManager::draw()
// {
//     for (Building* building: buildings)
//         building->draw();

//     for (Building* building: buildQueue)
//         building->draw();

//     if (ghostBuilding)
//     {
//         Cube& cube = ghostBuilding->getCube();
//         cube.position = Vector3AddValue(cube.position, 0.1f); // prevents z-fighting that occurs when buildings overlap
//         ghostBuilding->draw();
//         cube.position = Vector3SubtractValue(cube.position, 0.1f); // revert change
//     }
// }

// void BuildingManager::drawBuildingUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine)
// {
//     std::vector<ActionNode> children = ActionsManager::get().getActionChildren(selectedBuilding->actionId);

//     ActionNode fillerButton = ActionNode("filler", "Filler", "filler", {});
//     ActionNode sellButton = ActionNode("sell", "Sell", "sell", {});

//     int nrOfFillerButtons = nrOfButtons - children.size();
//     for (int i = 0; i < nrOfFillerButtons - 1; i++)
//         children.push_back(fillerButton);           // add filler buttons between actual buttons and sell button
//     if (nrOfFillerButtons)
//         children.push_back(sellButton);             // add sellButton last so its the last button

//     ActionNode child;
//     bool buttonWasPressed = false;
//     for (int i = 0; i < children.size(); i += buttonsPerLine)
//     {
//         for (int j = 0; j < buttonsPerLine; j++)
//         {
//             child = children[i+j];

//             if (child.id == "filler")
//                 ImGui::InvisibleButton(child.name.c_str(), buttonSize);
//             else
//             {
//                 int colors = canPromoteTo(child.id) ? pushButtonEnabled() : pushButtonDisabled();
//                 if (ImGui::Button(child.name.c_str(), buttonSize))
//                 {
//                     buttonWasPressed = true;
//                     resolveBuildingAction(selectedBuilding, child);
//                 }

//                 ImGui::PopStyleColor(colors); // remove pushed colors
//             }

//             if (j != buttonsPerLine - 1) // apply on all except the last
//                 ImGui::SameLine();
//         }
//     }

//     if (!buttonWasPressed) // check if any button was clicked using number-key buttons
//         for (int i = 0; i < children.size(); i++)
//             if (IsKeyPressed((KeyboardKey)int(KEY_ONE) + i))
//             {
//                 resolveBuildingAction(selectedBuilding, children[i]);
//                 break;
//             }
// }


// void BuildingManager::resolveBuildingAction(Building* building, ActionNode& node)
// {
//     if (node.id == "filler")
//         return;
//     else if (node.action == "sell")
//         building->sold = true;
//     else if (node.action == "recruit") // TODO: later // recruit(building);
//         printf("'recruit' action is not implemented\n");
//     else if (node.action == "buy") // TODO: later // player->tryBuyItem(Item(node.name));
//         printf("'buy' action is not implemented\n");
//     else if (node.action == "promote" && canPromoteTo(node.id))
//         promote(building, node.id);
// }

// Building* BuildingManager::raycastToBuilding()
// {
//     Ray ray = CameraManager::get().getMouseRay();
//     float closestCollisionDistance = std::numeric_limits<float>::infinity();
//     Building* nearestBuilding = nullptr;

//     for (Building* building: buildings)
//     {
//         Cube cube = building->getCube();
//         RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(cube));

//         if (collision.hit && collision.distance < closestCollisionDistance)
//         {
//             closestCollisionDistance = collision.distance;
//             nearestBuilding = building;
//         }
//     }

//     return nearestBuilding;
// }

// void BuildingManager::removeBuilding(Building* building)
// {
//     swapAndPop(buildings, building);

//     for (std::string& id: building->previousActionIds)
//         unlockedActions[id]--;
//     unlockedActions[building->actionId]--;

//     delete building;
// }

// void BuildingManager::update()
// {
//     for (Building* building: buildings)
//         building->update();

//     for (Building* building: buildQueue)
//         building->update();

//     if (ghostBuilding)
//         updateGhostBuilding();

//     if (selectedBuilding && selectedBuilding->sold) // delete selectedBuilding and pop from buildings vector
//     {
//         MapGenerator::get().removeObstacle(selectedBuilding->getCube());
//         removeBuilding(selectedBuilding);
//         selectedBuilding = nullptr;
//     }
// }

// Building* BuildingManager::yieldBuildQueue()
// {
//     assert(buildQueue.size() != 0);

//     Building* building = buildQueue.front();
//     buildQueue.pop_front();
//     building->build();
//     buildings.push_back(building);

//     return building;
// }

// Building* BuildingManager::buildQueueFront()
// {
//     return buildQueue.size() ? buildQueue.front() : nullptr;
// }

// void BuildingManager::clearBuildQueue()
// {
//     while(buildQueue.size())
//     {
//         delete buildQueue.back();
//         buildQueue.pop_back();
//     }
// }

// void BuildingManager::updateGhostBuilding()
// {

//     Cube* cubeHit = MapGenerator::get().raycastToGround();
//     if (!cubeHit)
//         return;

//     Vector3 cubeSize = MapGenerator::get().getCubeSize();
//     float ground = MapGenerator::get().getHeight();
//     const float max = 10000.f;
//     // Check mouse collision against a plane spanning from -max to max, with y the same as the cubeHit y-level
//     RayCollision collision = GetRayCollisionQuad(
//         CameraManager::get().getMouseRay(),
//         { -max, ground + (cubeHit->position.y + cubeSize.y/2), -max },
//         { -max, ground + (cubeHit->position.y + cubeSize.y/2),  max },
//         {  max, ground + (cubeHit->position.y + cubeSize.y/2),  max },
//         {  max, ground + (cubeHit->position.y + cubeSize.y/2), -max }
//     );

//     assert(collision.hit == true); // sanity check

//     // should be edge of cube or middle of cube depending on building size multiple of a cube
//     float cubeOffset = (int(defaultBuildingSize.x / cubeSize.x)) % 2 == 0
//         ? cubeSize.y / 2
//         : cubeSize.y;

//     Vector3 snapped = {
//         nearestIncrement(collision.point.x + cubeOffset, cubeSize.x),
//         cubeHit->position.y + cubeSize.y / 2 + defaultBuildingSize.y / 2, // don't need nearest here, just calculate
//         nearestIncrement(collision.point.z + cubeOffset, cubeSize.z)
//     };

//     Vector3 offset = {
//         (cubeSize.x - defaultBuildingSize.x) / 2.0f,
//         0.f,
//         (cubeSize.z - defaultBuildingSize.z) / 2.0f,
//     };
//     Vector3 final = Vector3Add(snapped, offset);

//     ghostBuilding->setPosition(final);

//     ghostBuildingIsColliding = (isColliding(buildings, ghostBuilding) || isColliding(buildQueue, ghostBuilding));
//     ghostBuilding->getCube().color = ghostBuildingIsColliding ? RED : ghostBuilding->getGhostColor();
// }

// template<typename Container>
// bool BuildingManager::isColliding(const Container& buildings, Building* targetBuilding)
// {
//     BoundingBox targetBoundingBox = getCubeBoundingBox(targetBuilding->getCube(), 0.8f);

//     for (Building* building: buildings)
//         if (CheckCollisionBoxes(targetBoundingBox, getCubeBoundingBox(building->getCube())))
//             return true;

//     return false;
// }

// void BuildingManager::createDebugBuilding(Vector2i index, BuildingType buildingType)
// {
//     Building* building = new Building(Cube(defaultBuildingSize), buildingType, nullptr);
//     unlockedActions[building->actionId]++;

//     MapGenerator& mapGenerator = MapGenerator::get();
//     Vector3 pos = mapGenerator.indexToWorldPosition(index);
//     Vector3 cubeSize = mapGenerator.getCubeSize();
//     Vector3 snapped = {
//         nearestIncrement(pos.x, cubeSize.x),
//         pos.y, // don't need nearest here
//         nearestIncrement(pos.z, cubeSize.z)
//     };
//     Vector3 offset = {
//         (cubeSize.x - defaultBuildingSize.x) / 2.0f,
//         0.f,
//         (cubeSize.z - defaultBuildingSize.z) / 2.0f,
//     };

//     building->setPosition(Vector3Add(snapped, offset));
//     mapGenerator.addObstacle(building->getCube());

//     building->scheduleBuild();
//     buildQueue.push_back(building);
//     yieldBuildQueue();
// }

// void BuildingManager::createNewGhostBuilding(BuildingType buildingType, Player* player)
// {
//     if (ghostBuilding)
//         delete ghostBuilding;

//     ghostBuilding = new Building(Cube(defaultBuildingSize), buildingType, player);
//     updateGhostBuilding(); // sets position correctly
// }

// void BuildingManager::clearGhostBuilding()
// {
//     if (ghostBuilding)
//     {
//         delete ghostBuilding;
//         ghostBuilding = nullptr;
//     }
// }

// void BuildingManager::scheduleGhostBuilding()
// {
//     assert(ghostBuilding != nullptr);

//     unlockedActions[ghostBuilding->actionId]++;
//     ghostBuilding->scheduleBuild();
//     buildQueue.push_back(ghostBuilding);
//     ghostBuilding = nullptr;
// }

// Building* BuildingManager::getGhostBuilding()
// {
//     return ghostBuilding;
// }

// bool BuildingManager::ghostBuildingExists()
// {
//     return ghostBuilding != nullptr;
// }

// void BuildingManager::select(Building* building)
// {
//     selectedBuilding = building;
//     selectedBuilding->select();
// }

// void BuildingManager::deselect()
// {
//     if (!selectedBuilding)
//         return;

//     selectedBuilding->deselect();
//     selectedBuilding = nullptr;
// }

// bool BuildingManager::canScheduleGhostBuilding()
// {
//     return !ghostBuildingIsColliding;
// }

// void BuildingManager::recruit(Building* building)
// {
//     MapGenerator& mapGenerator = MapGenerator::get();

//     std::vector<Vector2i> neighboringIndices = mapGenerator.getNeighboringIndices(building->getCube());
//     if (!neighboringIndices.size()) // no valid neighboring tiles
//     {
//         printf("Found no neighboring tiles, should probably do something about this later\n"); // TODO: later
//         return;
//     }

//     Vector3 neighborPosition = mapGenerator.indexToWorldPosition(neighboringIndices[0]);
//     float ground = building->cube.position.y;
//     Vector3 pos = { neighborPosition.x, ground, neighborPosition.z };
//     Vector3 speed = Vector3Scale(Vector3One(), 30);
//     // TEMP: disabled
//     // Entity* entity = new Entity(pos, speed, BLACK, WORKER);

//     // std::vector<Vector3> positions = mapGenerator.pathfindPositions(entity->getPosition(), building->getRallyPoint().position);
//     // entity->setPositions(positions);

//     // entities.push_back(entity);
// }

// bool BuildingManager::canPromoteTo(std::string id)
// {
//     std::vector<std::string> requirements = ActionsManager::get().requirements[id];
//     for (const std::string& id: requirements)
//         if (unlockedActions[id] == 0) // defaults to 0 if it doesn't exist
//             return false;

//     return true;
// }

// void BuildingManager::promote(Building* building, std::string id)
// {
//     printf("promote %s to %s\n", building->actionId.c_str(), id.c_str());

//     unlockedActions[id]++;
//     building->previousActionIds.push_back(building->actionId);
//     building->actionId = id;
// }
