#include "GameScreen.h"

GameScreen::GameScreen(Vector2i screenSize)
{
    this->screenSize = screenSize;

    Vector3 cubeSize = { 4.f, 4.f, 4.f };
    Vector2i gridSize = { 32, 32 };

    layer = new Layer();
    layer->createGrid(gridSize, cubeSize, DARKGRAY, 0.f);

    selectedBuilding = nullptr;
    buildingManager = new BuildingManager({ cubeSize.x * 2, cubeSize.y, cubeSize.z * 2 }, WHITE, layer);

    buildingTypeMappings = {
        { CASTLE, { KEY_ONE, "Castle" } },
        { ROCK, { KEY_TWO, "Rock" } },
    };

    Vector3 startPos = { 0.f, cubeSize.y, 0.f };
    Vector3 endPos = { 0.f, cubeSize.y + 8.f, 0.f };
    startPos.x = endPos.x = gridSize.x/2 * cubeSize.x - cubeSize.x;
    startPos.z = endPos.z = gridSize.y/2 * cubeSize.z - cubeSize.z;
    float radius = 2.f;
    int slices = 16;
    int rings = 4;
    Color playerColor = BLUE;
    Vector3 playerSpeed = Vector3Scale(Vector3One(), 40);
    printVector3("playerSpeed", playerSpeed);
    player = new Player(Capsule(startPos, endPos, radius, slices, rings, playerColor), playerSpeed);

    isMultiSelecting = false;
}

GameScreen::~GameScreen()
{
    if (layer)
        delete layer;

    if (buildingManager)
        delete buildingManager;

    if (player)
        delete player;
}

void GameScreen::draw()
{
    Camera3D& camera3D = CameraManager::get().getCamera();
    Camera2D& camera2D = CameraManager::get().getCamera2D();

    BeginMode3D(camera3D);

        layer->draw();

        if (buildingManager)
            buildingManager->draw();

        if (player)
            player->draw();

        drawUI();

    EndMode3D();

    BeginMode2D(camera2D);

        if (isMultiSelecting)
        {
            DrawRectangleRec(multiSelectionRectangle, { 0, 255, 0, 25 });
            DrawRectangleLinesEx(multiSelectionRectangle, 1.f, { 0, 255, 0, 50 });

            std::vector<Entity*> entities = buildingManager->getEntities();
            entities.push_back(player);

            if (true) // set to true to draw all entities' capsule collision circles
            {
                for (Entity* entity: entities)
                {
                    Capsule cap = entity->getCapsule();

                    Vector2 bottomCirclePosScreen = GetWorldToScreen(cap.startPos, camera3D);
                    Vector2 topCirclePosScreen = GetWorldToScreen(cap.endPos, camera3D);

                    float newBottomRadius = calculateCircleRadius2D(cap.startPos, cap.radius);
                    float newTopRadius = calculateCircleRadius2D(cap.endPos, cap.radius);

                    DrawCircleV(bottomCirclePosScreen, newBottomRadius, YELLOW);
                    DrawCircleLinesV(bottomCirclePosScreen, newBottomRadius, GRAY);

                    DrawCircleV(topCirclePosScreen, newTopRadius, YELLOW);
                    DrawCircleLinesV(topCirclePosScreen, newTopRadius, GRAY);
                }
            }
        }

    EndMode2D();
}

void GameScreen::drawUI()
{
    bool playerSelectedAndNoOther = (selectedEntities.size() == 1 && player->isSelected());
    bool drawWindow = (!selectedBuilding != !playerSelectedAndNoOther); // xor
    if (drawWindow)
    {
        bool bottomRightWindow = true;
        int bottomRightWindowFlags = 0;
        bottomRightWindowFlags |= ImGuiWindowFlags_NoTitleBar;
        bottomRightWindowFlags |= ImGuiWindowFlags_NoResize;
        bottomRightWindowFlags |= ImGuiWindowFlags_NoMove;

        ImVec2 windowSize(200, 200);
        ImVec2 windowPos(
            screenSize.x - windowSize.x,
            screenSize.y - windowSize.y
        );

        ImVec2 windowPadding(8, 8);
        ImVec2 buttonPadding(8, 8);
        ImVec2 buttonSize(
            windowSize.x / 2 - windowPadding.x - buttonPadding.x / 2,
            windowSize.y / 2 - windowPadding.y - buttonPadding.x / 2
        );

        // Draw bottom right window
        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(windowPos);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);
        ImGui::Begin("Bottom right window", &bottomRightWindow, bottomRightWindowFlags);
        ImGui::PopStyleVar();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, buttonPadding);

        if (selectedBuilding)
            selectedBuilding->drawUIButtons(windowPadding, buttonSize);
        else if (player->isSelected())
        {
            int buttonsPerLine = 2;
            for (int i = 1; i < BuildingType::COUNT; i += buttonsPerLine) // start at 1 since 0 = NONE
            {
                for (int j = 0; j < buttonsPerLine; j++)
                {
                    if ((BuildingType)(i+j) == BuildingType::COUNT) // safeguard if not a multiple of buttonsPerLine
                        break;

                    BuildingType buildingType = (BuildingType)(i+j);
                    UIMapping mapping = buildingTypeMappings[buildingType];

                    if (ImGui::Button(mapping.buttonText.c_str(), buttonSize))
                        buildingManager->createNewGhostBuilding(buildingType);

                    if (j != buttonsPerLine - 1) ImGui::SameLine(); // apply on all except the last
                }
            }
        }

        ImGui::PopStyleVar();
        ImGui::End();
    }

}

void GameScreen::update()
{
    CameraManager::get().update();
    buildingManager->update();
    player->update();

    if (buildingManager->buildQueueFront()) // something is getting built
    {
        if (player->getState() == IDLE && player->getPreviousState() == RUNNING_TO_BUILD) // was running and reached target
        {
            Building* building = buildingManager->yieldBuildQueue();
            layer->addObstacle(building->getCube());

            building = buildingManager->buildQueueFront();
            if (building) // if more in queue, walk to the next target
            {
                Vector3 targetPosition = calculateTargetPositionToBuildingFromPlayer(building);
                std::vector<Vector3> positions = layer->pathfindPositions(player->getPosition(), targetPosition);
                player->setPositions(positions, RUNNING_TO_BUILD);
            }
        }
    }

    if (selectedBuilding && selectedBuilding->isSold()) // delete selectedBuilding and pop from buildings vector
    {
        layer->removeObstacle(selectedBuilding->getCube());
        buildingManager->removeBuilding(selectedBuilding);
        selectedBuilding = nullptr;
    }

    bool wasMultiSelecting = isMultiSelecting;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))                        // LMB was clicked this frame
        handleLeftMouseButton();                                        // this may update isMultiSelecting
    else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isMultiSelecting)  // keep updating rectangle
        updateMultiSelectionRectangle();

    if (!wasMultiSelecting && isMultiSelecting) // update only first time isMultiSelecting is set to true
    {
        multiSelectionStartPosition = GetMousePosition();
        updateMultiSelectionRectangle();
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && isMultiSelecting) // stop multi selection
    {
        isMultiSelecting = false;

        clearAndDeselectAllSelectedEntities();
        std::vector<Entity*> entities = buildingManager->getEntities();
        entities.push_back(player); // add player to skip extra code to deal with him

        for (Entity* entity: entities)
        {
            if (checkCollisionCapsuleRectangle(entity->getCapsule(), multiSelectionRectangle)) // entity is inside multiSelectionRectangle
            {
                entity->select();
                selectedEntities.push_back(entity);
            }
        }
    }

    if (selectedEntities.size() == 1 && player->isSelected()) // Check additional mapping keys
    {
        for (int i = 1; i < BuildingType::COUNT; i += 1) // start at 1 since 0 = NONE
        {
            BuildingType buildingType = (BuildingType)i;
            UIMapping mapping = buildingTypeMappings[buildingType];

            if (IsKeyPressed(mapping.key))
            {
                buildingManager->createNewGhostBuilding(buildingType);
                break;
            }
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        handleRightMouseButton();
}

float GameScreen::calculateCircleRadius2D(Vector3 position, float radius)
{
    Matrix viewMatrix = CameraManager::get().getCameraViewMatrix();

    Vector3 right = { viewMatrix.m0, viewMatrix.m1, viewMatrix.m2 };
    Vector3 rightScaled = Vector3Scale(right, radius);
    Vector3 edgeOfCircle = Vector3Add(position, rightScaled);

    return Vector2Distance(
        CameraManager::get().getWorldToScreen(position),
        CameraManager::get().getWorldToScreen(edgeOfCircle)
    );
}

bool GameScreen::checkCollisionCapsuleRectangle(Capsule capsule, Rectangle rectangle)
{
    Vector2 bottomCirclePosScreen = CameraManager::get().getWorldToScreen(capsule.startPos);
    Vector2 topCirclePosScreen = CameraManager::get().getWorldToScreen(capsule.endPos);

    float newBottomRadius = calculateCircleRadius2D(capsule.startPos, capsule.radius);
    float newTopRadius = calculateCircleRadius2D(capsule.endPos, capsule.radius);

    // TODO: add collision against cylinder bounding box lines
    if (CheckCollisionCircleRec(bottomCirclePosScreen, newBottomRadius, rectangle)  // check against bottom circle
    ||  CheckCollisionCircleRec(topCirclePosScreen, newTopRadius, rectangle))       // check against top circle
        return true;

    return false;
}

bool GameScreen::checkCollisionCapsulePoint(Capsule capsule, Vector2 point)
{
    Vector2 bottomCirclePosScreen = CameraManager::get().getWorldToScreen(capsule.startPos);
    Vector2 topCirclePosScreen = CameraManager::get().getWorldToScreen(capsule.endPos);

    float newBottomRadius = calculateCircleRadius2D(capsule.startPos, capsule.radius);
    float newTopRadius = calculateCircleRadius2D(capsule.endPos, capsule.radius);

    // TODO: add collision against cylinder bounding box
    if (CheckCollisionPointCircle(point, bottomCirclePosScreen, newBottomRadius)  // check against bottom circle
    ||  CheckCollisionPointCircle(point, topCirclePosScreen, newTopRadius))       // check against top circle
        return true;

    return false;
}

void GameScreen::updateMultiSelectionRectangle()
{
    Vector2 mousePos = GetMousePosition();
    Vector2 direction = Vector2Subtract(mousePos, multiSelectionStartPosition);

    if (direction.y >= 0) // draw from multiSelectionStartPosition.y to direction.y
    {
        multiSelectionRectangle.y = multiSelectionStartPosition.y;
        multiSelectionRectangle.height = direction.y;
    }
    else // draw from mousePos.y to -direction.y instead, raylib doesn't like negative sizes
    {
        multiSelectionRectangle.y = mousePos.y;
        multiSelectionRectangle.height = -direction.y;
    }

    if (direction.x >= 0) // draw from multiSelectionStartPosition.x to direction.x
    {
        multiSelectionRectangle.x = multiSelectionStartPosition.x;
        multiSelectionRectangle.width = direction.x;
    }
    else // draw from mousePos.x to -direction.x instead, raylib doesn't like negative sizes
    {
        multiSelectionRectangle.x = mousePos.x;
        multiSelectionRectangle.width = -direction.x;
    }
}

RaycastHitType GameScreen::checkRaycastHitType()
{
    if (ImGui::GetIO().WantCaptureMouse)
        return RAYCAST_HIT_TYPE_UI;

    if (raycastToPlayer())
        return RAYCAST_HIT_TYPE_PLAYER;

    if (raycastToEntity())
        return RAYCAST_HIT_TYPE_ENTITY;

    if (buildingManager->raycastToBuilding())
        return RAYCAST_HIT_TYPE_BUILDING;

    RayCollision ground = raycastToGround();
    if (ground.hit && layer->worldPositionWithinBounds(ground.point))
        return RAYCAST_HIT_TYPE_GROUND;

    return RAYCAST_HIT_TYPE_OUT_OF_BOUNDS;
}

void GameScreen::handleLeftMouseButton()
{
    RaycastHitType type = checkRaycastHitType();

    // deselect selectedBuilding if not clicking a building or UI
    if (selectedBuilding && type != RAYCAST_HIT_TYPE_UI && type != RAYCAST_HIT_TYPE_BUILDING)
    {
        selectedBuilding->deselect();
        selectedBuilding = nullptr;
    }

    // always clear selectetedEntities if clicked player/entity/building
    if (type == RAYCAST_HIT_TYPE_PLAYER || type == RAYCAST_HIT_TYPE_ENTITY || type == RAYCAST_HIT_TYPE_BUILDING)
        clearAndDeselectAllSelectedEntities();

    switch (type)
    {
        case RAYCAST_HIT_TYPE_UI:
            break;

        case RAYCAST_HIT_TYPE_OUT_OF_BOUNDS:
            isMultiSelecting = true;
            break;

        case RAYCAST_HIT_TYPE_PLAYER:
            selectedEntities.push_back(player);
            player->select();
            break;

        case RAYCAST_HIT_TYPE_ENTITY:
        {
            Entity* entity = raycastToEntity();
            selectedEntities.push_back(entity);
            entity->select();
            break;
        }

        case RAYCAST_HIT_TYPE_BUILDING:
        {
            Building* building = buildingManager->raycastToBuilding();
            if (!selectedBuilding) // new select
            {
                selectedBuilding = building;
                selectedBuilding->select();
                break;
            }

            if (building != selectedBuilding) // switch select
            {
                selectedBuilding->deselect();
                selectedBuilding = building;
                selectedBuilding->select();
            }
            break;
        }

        case RAYCAST_HIT_TYPE_GROUND:
        {
            if (player->isSelected() && buildingManager->ghostBuildingExists())
            {
                if (!buildingManager->canScheduleGhostBuilding()) // can't schedule ghostbuilding
                    break;

                if (player->getState() == RUNNING_TO_BUILD) // already building, just schedule and leave player unchanged
                {
                    buildingManager->scheduleGhostBuilding();
                    break;
                }

                // run to new target and schedule building
                Vector3 targetPosition = calculateTargetPositionToBuildingFromPlayer(buildingManager->getGhostBuilding());
                std::vector<Vector3> positions = layer->pathfindPositions(player->getPosition(), targetPosition);
                player->setPositions(positions, RUNNING_TO_BUILD);
                buildingManager->scheduleGhostBuilding();
                break;
            }

            // player is not trying to place a ghostbuilding
            if (player->isSelected())
                player->deselect();

            isMultiSelecting = true; // start multi selection
            break;
        }
    }
}

void GameScreen::handleRightMouseButton()
{
    RaycastHitType type = checkRaycastHitType();

    switch (type)
    {
        case RAYCAST_HIT_TYPE_GROUND:
        {
            if (selectedEntities.size())
                handleRightMouseButtonWithEntity();
            else if (selectedBuilding)
                handleRightMouseButtonWithBuilding();

            break;
        }

        case RAYCAST_HIT_TYPE_UI:               // do nothing
        case RAYCAST_HIT_TYPE_PLAYER:           // do nothing
        case RAYCAST_HIT_TYPE_ENTITY:           // do nothing
        case RAYCAST_HIT_TYPE_BUILDING:         // do nothing
        case RAYCAST_HIT_TYPE_OUT_OF_BOUNDS:    // do nothing
            break;
    }
}

void GameScreen::handleRightMouseButtonWithEntity()
{
    if (player->isSelected())
    {
        if (buildingManager->ghostBuildingExists()) // remove building and just run the player to the location instead
            buildingManager->clearGhostBuilding();

        if (player->getState() == RUNNING_TO_BUILD) // was running to build something, clear queue
            buildingManager->clearBuildQueue();
    }

    // run all selected entities to where mouse was clicked
    Vector3 goal = raycastToGround().point;
    Vector2i goalIndex = layer->worldPositionToIndex(goal);
    std::vector<Vector2i> neighboringIndices = layer->getNeighboringIndices({ goalIndex });

    neighboringIndices.insert(neighboringIndices.begin(), goalIndex); // insert at front so its guaranteed to be picked

    std::vector<Vector3> neighboringPositions;
    for (Vector2i index: neighboringIndices)
        neighboringPositions.push_back(layer->indexToWorldPosition(index));

    if (selectedEntities.size() > neighboringIndices.size())
        printf("entities > indices, should probably do something about this later\n"); // TODO: later

    Entity* entity = nullptr;
    for (int i = 0; i < selectedEntities.size(); i++)
    {
        Vector3 pos = layer->indexToWorldPosition(neighboringIndices[i]);
        entity = selectedEntities[i];
        entity->setPositions(layer->pathfindPositions(entity->getPosition(), pos), RUNNING);
    }

    if (false) // set to true to color neighboring tiles
    {
        std::list<Vector2i> listIndices;
        listIndices.insert(listIndices.begin(), neighboringIndices.begin(), neighboringIndices.end());
        layer->colorTiles(listIndices);
    }
}

void GameScreen::handleRightMouseButtonWithBuilding()
{
    Vector3 point = raycastToGround().point;
    Vector2i index = layer->worldPositionToIndex(point);
    Vector3 adjusted = layer->indexToWorldPosition(index);
    selectedBuilding->setRallyPoint(adjusted);
}

bool GameScreen::raycastToPlayer()
{
    return checkCollisionCapsulePoint(player->getCapsule(), GetMousePosition());
}

Entity* GameScreen::raycastToEntity()
{
    std::vector<Entity*> entities = buildingManager->getEntities();
    for (Entity* entity: entities)
        if (checkCollisionCapsulePoint(entity->getCapsule(), GetMousePosition()))
            return entity;

    return nullptr;
}

RayCollision GameScreen::raycastToGround()
{
    float ground = layer->getHeight();
    const float max = 10000.f;
    // Check mouse collision against a plane spanning from -max to max, with y the same as the ground level
    return GetRayCollisionQuad(
        CameraManager::get().getMouseRay(),
        { -max, ground, -max },
        { -max, ground,  max },
        {  max, ground,  max },
        {  max, ground, -max }
    );
}

void GameScreen::clearAndDeselectAllSelectedEntities()
{
    for (Entity* entity: selectedEntities)
        entity->deselect();

    selectedEntities.clear();
}

Vector3 GameScreen::calculateTargetPositionToBuildingFromPlayer(Building* building)
{
    std::vector<Vector2i> indices = layer->getNeighboringIndices(building->getCube());
    std::vector<Vector3> positions;
    Vector3 playerPosition = player->getPosition();
    Vector3 position;
    for (Vector2i index: indices)
    {
        position = layer->indexToWorldPosition(index);
        if (position.x == playerPosition.x && position.z == playerPosition.z)
            return position;

        positions.push_back(position);
    }

    if (positions.empty())
    {
        printf("Found no valid positions, should probably do something about this later\n"); // TODO: later
        return Vector3Zero();
    }

    return positions[0]; // just grab the first one, don't care which one right now
}
