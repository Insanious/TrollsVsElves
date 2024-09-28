#include "GameScreen.h"

GameScreen::GameScreen(Vector2i screenSize)
{
    this->screenSize = screenSize;
    ActionsManager::get().loadRequirements("requirements.json");
    ActionsManager::get().loadActions("actions.json");

    MapGenerator::get().generateFromFile("map/map.json");
    Vector2i gridSize = MapGenerator::get().getGridSize();
    Vector3 cubeSize = MapGenerator::get().getCubeSize();

    selectedBuilding = nullptr;
    buildingManager = new BuildingManager({ cubeSize.x * 2, cubeSize.y, cubeSize.z * 2 }, BLANK);

    Vector3 startPos = { 0.f, cubeSize.y / 2, 0.f };
    startPos.x = gridSize.x / 2 * cubeSize.x - cubeSize.x; // spawn in corner
    startPos.z = gridSize.y / 2 * cubeSize.z - cubeSize.z; // spawn in corner
    Vector3 playerSpeed = Vector3Scale(Vector3One(), 40);
    player = new Player(startPos, playerSpeed, PLAYER_ELF);

    player->setBuildingManager(buildingManager);

    isMultiSelecting = false;

    lastLeftMouseButtonClick = std::chrono::steady_clock::now();

    // add some "trees"
    Vector2 halfGridSize = { gridSize.x/2 * cubeSize.x, gridSize.y/2 * cubeSize.y };
    for (int z = gridSize.y/2; z < gridSize.y - gridSize.y/8; z++)
    {
        Vector3 pos = { -halfGridSize.x, 0.f, cubeSize.z * z - halfGridSize.y, };
        addResource(pos);
    }

    buildingManager->createDebugBuilding({ 15, 15 }, ROCK);
}

GameScreen::~GameScreen()
{
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

        MapGenerator::get().draw();

        for (Resource* resource: resources)
            resource->draw();

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

            // std::vector<Entity*> entities = buildingManager->getEntities(); // TEMP
            std::vector<Entity*> entities;
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

        if (selectedBuilding)
            buildingManager->drawBuildingUIButtons(selectedBuilding, buttonSize, nrOfButtons, buttonLayout.x);
        else if (player->isSelected())
            player->drawUIButtons(buttonSize, nrOfButtons, buttonLayout.x);

        ImGui::PopStyleVar();
        ImGui::End();
    }

}

void GameScreen::update()
{
    CameraManager::get().update();
    buildingManager->update();
    player->update();

    if (player->hasReachedDestination() && buildingManager->buildQueueFront()) // something is getting built
    {
        Building* building = buildingManager->yieldBuildQueue();
        MapGenerator::get().addObstacle(building->getCube());

        building = buildingManager->buildQueueFront();
        if (building) // if more in queue, walk to the next target
        {
            Vector3 targetPosition = calculateTargetPositionToCubeFromEntity(player, building->getCube());
            std::vector<Vector3> positions = MapGenerator::get().pathfindPositions(player->getPosition(), targetPosition);
            player->setPositions(positions);
        }
    }

    if (selectedBuilding && selectedBuilding->sold) // delete selectedBuilding and pop from buildings vector
    {
        MapGenerator::get().removeObstacle(selectedBuilding->getCube());
        buildingManager->removeBuilding(selectedBuilding);
        selectedBuilding = nullptr;
    }

    if (!isMultiSelecting)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) // LMB was clicked this frame
            handleLeftMouseButton();
    }
    else
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            updateMultiSelection();
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            stopMultiSelection();
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

void GameScreen::startMultiSelection()
{
    isMultiSelecting = true;
    multiSelectionStartPosition = GetMousePosition();
    updateMultiSelection();
}

void GameScreen::stopMultiSelection()
{
    isMultiSelecting = false;

    clearAndDeselectAllSelectedEntities();
    std::vector<Entity*> entities;
    // std::vector<Entity*> entities = buildingManager->getEntities(); // TEMP
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

void GameScreen::updateMultiSelection()
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

    if (raycastToResource())
        return RAYCAST_HIT_TYPE_RESOURCE;

    if (buildingManager->raycastToBuilding())
        return RAYCAST_HIT_TYPE_BUILDING;

    if (MapGenerator::get().raycastToGround())
        return RAYCAST_HIT_TYPE_GROUND;

    return RAYCAST_HIT_TYPE_OUT_OF_BOUNDS;
}

void GameScreen::handleLeftMouseButton()
{
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsedTime = now - lastLeftMouseButtonClick;
    bool doubleclicked = elapsedTime.count() < 0.2f;
    lastLeftMouseButtonClick = now;

    RaycastHitType type = checkRaycastHitType();

    // deselect selectedBuilding if not clicking a building or UI
    if (selectedBuilding && type != RAYCAST_HIT_TYPE_UI)
    {
        selectedBuilding->deselect();
        selectedBuilding = nullptr;
    }

    // always clear selectedEntities if clicked player/entity/building
    if (type == RAYCAST_HIT_TYPE_PLAYER || type == RAYCAST_HIT_TYPE_ENTITY || type == RAYCAST_HIT_TYPE_BUILDING)
        clearAndDeselectAllSelectedEntities();

    switch (type)
    {
        case RAYCAST_HIT_TYPE_UI:
            break;

        case RAYCAST_HIT_TYPE_OUT_OF_BOUNDS:
            startMultiSelection();
            break;

        case RAYCAST_HIT_TYPE_PLAYER:
            selectedEntities.push_back(player);
            player->select();
            break;

        case RAYCAST_HIT_TYPE_ENTITY:
        {
            // TEMP
            // Entity* clickedEntity = raycastToEntity();
            // if (doubleclicked) // select all of the same type
            // {
            //     std::vector<Entity*> entities = buildingManager->getEntities();
            //     EntityType clickedEntityType = clickedEntity->getEntityType();
            //     for (Entity* entity: entities)
            //     {
            //         if (entity->getEntityType() == clickedEntityType)
            //         {
            //             selectedEntities.push_back(entity);
            //             entity->select();
            //         }
            //     }
            // }
            // else
            // {
            //     selectedEntities.push_back(clickedEntity);
            //     clickedEntity->select();
            // }

            break;
        }

        case RAYCAST_HIT_TYPE_BUILDING:
        {
            selectedBuilding = buildingManager->raycastToBuilding();
            selectedBuilding->select();
            break;
        }

        case RAYCAST_HIT_TYPE_GROUND:
        {
            if (player->isSelected() && buildingManager->ghostBuildingExists())
            {
                if (!buildingManager->canScheduleGhostBuilding()) // can't schedule ghostbuilding
                    break;

                Building* ghost = buildingManager->getGhostBuilding();
                bool buildingsInQueue = buildingManager->buildQueueFront() != nullptr;
                buildingManager->scheduleGhostBuilding();
                if (buildingsInQueue) // something is getting built, just schedule and leave player unchanged
                    break;

                // run to new target
                Vector3 targetPosition = calculateTargetPositionToCubeFromEntity(player, ghost->getCube());
                std::vector<Vector3> positions = MapGenerator::get().pathfindPositions(player->getPosition(), targetPosition);
                player->setPositions(positions);
                break;
            }

            if (player->isSelected()) // player is not trying to place a ghostbuilding
                player->deselect();

            if (!isMultiSelecting)
                startMultiSelection();

            break;
        }
    }
}

void GameScreen::handleRightMouseButton()
{
    MapGenerator& mapGenerator = MapGenerator::get();
    RaycastHitType type = checkRaycastHitType();

    switch (type)
    {
        case RAYCAST_HIT_TYPE_GROUND:
        {
            if (selectedEntities.size())
            {
                if (player->isSelected())
                {
                    buildingManager->clearGhostBuilding();
                    buildingManager->clearBuildQueue();
                }

                // run all selected entities to where mouse was clicked
                Vector3 goal = mapGenerator.raycastToGround()->position;
                Vector2i goalIndex = mapGenerator.worldPositionToIndex(goal);
                std::vector<Vector2i> neighboringIndices = mapGenerator.getNeighboringIndices({ goalIndex });

                neighboringIndices.insert(neighboringIndices.begin(), goalIndex); // insert at front so its guaranteed to be picked

                if (selectedEntities.size() > neighboringIndices.size())
                    printf("entities > indices, should probably do something about this later\n"); // TODO: later

                Entity* entity = nullptr;
                bool checkForTroll = player->playerType == PLAYER_TROLL && player->isSelected();
                for (int i = 0; i < selectedEntities.size(); i++)
                {
                    Vector3 pos = mapGenerator.indexToWorldPosition(neighboringIndices[i]);
                    entity = selectedEntities[i];

                    if (checkForTroll && entity == player) // TODO: later, this seems inefficient but good enough for now
                        entity->setPositions(mapGenerator.pathfindPositionsForTroll(entity->getPosition(), pos));
                    else
                        entity->setPositions(mapGenerator.pathfindPositions(entity->getPosition(), pos));
                    entity->detach();
                }

                if (false) // set to true to color neighboring tiles
                {
                    std::list<Vector2i> listIndices;
                    listIndices.insert(listIndices.begin(), neighboringIndices.begin(), neighboringIndices.end());
                    mapGenerator.colorTiles(listIndices);
                }

                break;
            }

            if (selectedBuilding)
            {
                Vector3 point = raycastToGround().point;
                Vector2i index = mapGenerator.worldPositionToIndex(point);
                Vector3 adjusted = mapGenerator.indexToWorldPosition(index);
                selectedBuilding->setRallyPoint(adjusted); // TODO: later, this doesn't work for y-elevated buildings

                break;
            }

            break;
        }

        case RAYCAST_HIT_TYPE_BUILDING:
        {
            if (!player->isSelected() && selectedEntities.size())
            {
                Building* building = buildingManager->raycastToBuilding();

                Vector3 targetPosition;
                std::vector<Vector3> positions;
                for (Entity* entity: selectedEntities)
                {
                    targetPosition = calculateTargetPositionToCubeFromEntity(entity, building->getCube());
                    positions = mapGenerator.pathfindPositions(entity->getPosition(), targetPosition);
                    entity->setPositions(positions);

                    entity->attach(building);
                }
            }

            break;
        }

        case RAYCAST_HIT_TYPE_RESOURCE:
        {
            if (!player->isSelected() && selectedEntities.size())
            {
                Resource* resource = raycastToResource();

                Vector3 targetPosition;
                std::vector<Vector3> positions;
                for (Entity* entity: selectedEntities)
                {
                    targetPosition = calculateTargetPositionToCubeFromEntity(entity, resource->getCube());
                    positions = mapGenerator.pathfindPositions(entity->getPosition(), targetPosition);
                    entity->setPositions(positions);

                    entity->attach(resource);
                }
            }
            break;
        }

        case RAYCAST_HIT_TYPE_UI:               // do nothing
        case RAYCAST_HIT_TYPE_PLAYER:           // do nothing
        case RAYCAST_HIT_TYPE_ENTITY:           // do nothing
        case RAYCAST_HIT_TYPE_OUT_OF_BOUNDS:    // do nothing
            break;
    }
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

Resource* GameScreen::raycastToResource()
{
    Ray ray = CameraManager::get().getMouseRay();
    float closestCollisionDistance = std::numeric_limits<float>::infinity();
    Resource* nearestResource = nullptr;

    for (Resource* resource: resources)
    {
        Cube cube = resource->getCube();
        RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(cube));

        if (collision.hit && collision.distance < closestCollisionDistance)
        {
            closestCollisionDistance = collision.distance;
            nearestResource = resource;
        }
    }

    return nearestResource;
}

RayCollision GameScreen::raycastToGround()
{
    float ground = MapGenerator::get().getHeight();
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

Vector3 GameScreen::calculateTargetPositionToCubeFromEntity(Entity* entity, Cube cube)
{
    MapGenerator& mapGenerator = MapGenerator::get();
    std::vector<Vector2i> indices = mapGenerator.getNeighboringIndices(cube);
    Vector3 entityPosition = entity->getPosition();

    std::vector<Vector3> positions;
    Vector3 position;
    for (Vector2i index: indices)
    {
        position = mapGenerator.indexToWorldPosition(index);
        if (position.x == entityPosition.x && position.z == entityPosition.z)
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

void GameScreen::addResource(Vector3 position)
{
    Vector3 size = { 2.f, 20.f, 2.f };
    Vector3 adjustedPosition = { position.x, position.y + size.y/2, position.z };

    Cube cube = { adjustedPosition, size, BLANK };
    Resource* resource = new Resource(cube);
    MapGenerator::get().addObstacle(cube);
    resources.push_back(resource);
}