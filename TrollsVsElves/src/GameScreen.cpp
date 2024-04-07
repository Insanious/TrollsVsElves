#include "GameScreen.h"

GameScreen::GameScreen() {}

GameScreen::~GameScreen()
{
    if (layer)
        delete layer;

    if (buildingManager)
        delete buildingManager;

    if (player)
        delete player;
}

void GameScreen::init(Vector2i screenSize)
{
    this->screenSize = screenSize;

    cubeSize = { 4.f, 4.f, 4.f };
    gridSize = { 32, 32 };
    defaultCubeColor = DARKGRAY;

    layer = new Layer();
    layer->createGrid(gridSize, cubeSize, DARKGRAY, 0.f);

    selectedBuilding = nullptr;
    buildingManager = new BuildingManager({ cubeSize.x * 2, cubeSize.y, cubeSize.z * 2 }, WHITE, layer);

    hoveringUI = false;

    buildingTypeMappings = {
        { CASTLE, { KEY_ONE, "Castle" } },
        { ROCK, { KEY_TWO, "Rock" } },
    };

    player = new Player();
    Vector3 startPos = { 0.f, cubeSize.y, 0.f };
    Vector3 endPos = { 0.f, cubeSize.y + 8.f, 0.f };
    startPos.x = endPos.x = gridSize.x/2 * cubeSize.x;
    startPos.z = endPos.z = gridSize.y/2 * cubeSize.z;
    float radius = 2.f;
    int slices = 16;
    int rings = 16;
    Color playerColor = BLUE;
    Vector3 playerSpeed = Vector3Scale(Vector3One(), 40);
    player->init(Capsule(startPos, endPos, radius, slices, rings, playerColor), playerSpeed);

    canSelect = true;
    isSelecting = false;
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

        if (isSelecting)
        {
            DrawRectangleRec(selectionRectangle, { 0, 255, 0, 25 });
            DrawRectangleLinesEx(selectionRectangle, 1.f, { 0, 255, 0, 50 });

            if (true) // set to true to draw player capsule collision circles
            {
                Capsule cap = player->getCapsule();

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

    EndMode2D();
}

void GameScreen::drawUI()
{
    hoveringUI = false;

    bool drawWindow = !selectedBuilding != !player->isSelected(); // xor
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

        hoveringUI = ImGui::IsMouseHoveringRect(
            windowPos,
            ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y)
        );

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
                    if ((BuildingType)(i+j) == BuildingType::COUNT) break; // safeguard if not a multiple of buttonsPerLine

                    BuildingType buildingType = (BuildingType)(i+j);
                    UIMapping mapping = buildingTypeMappings[buildingType];
                    if (ImGui::Button(mapping.buttonText.c_str(), buttonSize))
                    {
                        buildingManager->createNewGhostBuilding(buildingType);
                        canSelect = false;
                    }

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
                std::vector<Vector3> positions = pathfindPositions(player->getPosition(), targetPosition);
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

    if (player->isSelected()) // Check additional mapping keys
    {
        for (int i = 1; i < BuildingType::COUNT; i += 1) // start at 1 since 0 = NONE
        {
            BuildingType buildingType = (BuildingType)i;
            UIMapping mapping = buildingTypeMappings[buildingType];

            if (IsKeyPressed(mapping.key))
            {
                buildingManager->createNewGhostBuilding(buildingType);
                canSelect = false;
            }
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        handleRightMouseButton();

    if (!canSelect && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        canSelect = true;

    bool leftMouseButtonWasPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (leftMouseButtonWasPressed)
        handleLeftMouseButton();

    if (canSelect && !hoveringUI && !buildingManager->ghostBuildingExists())
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !leftMouseButtonWasPressed)
        {
            if (!isSelecting)
            {
                isSelecting = true;
                selectionStartPosition = GetMousePosition();
            }

            updateSelectionRectangle();
        }
        else
        {
            if (isSelecting)
            {
                isSelecting = false;

                if (checkCollisionCapsuleRectangle(player->getCapsule(), selectionRectangle))
                {
                    player->select();
                    printf("selecting player\n");

                    if (selectedBuilding) // TODO: figure out logic for this guy...
                    {
                        selectedBuilding->deselect();
                        selectedBuilding = nullptr;
                    }
                }
                else
                {
                    player->deselect();
                }
            }
        }
    }
}

void GameScreen::updateSelectedBuilding()
{
    Building* building = buildingManager->raycastToBuilding();
    if (!building && !selectedBuilding)
        return;

    if (!building && selectedBuilding) // remove select
    {
        selectedBuilding->deselect();
        selectedBuilding = nullptr;

        player->deselect();
        return;
    }

    if (!selectedBuilding) // new select
    {
        selectedBuilding = building;
        selectedBuilding->select();

        player->deselect();
        canSelect = false;
        return;
    }

    if (selectedBuilding && selectedBuilding != building) // switch select
    {
        selectedBuilding->deselect();
        selectedBuilding = building;
        selectedBuilding->select();

        canSelect = false;
        return;
    }
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

void GameScreen::updateSelectionRectangle()
{
    Vector2 mousePos = GetMousePosition();
    Vector2 direction = Vector2Subtract(mousePos, selectionStartPosition);

    if (direction.y >= 0) // draw from selectionStartPosition.y to direction.y
    {
        selectionRectangle.y = selectionStartPosition.y;
        selectionRectangle.height = direction.y;
    }
    else // draw from mousePos.y to -direction.y instead, raylib doesn't like negative sizes
    {
        selectionRectangle.y = mousePos.y;
        selectionRectangle.height = -direction.y;
    }

    if (direction.x >= 0) // draw from selectionStartPosition.x to direction.x
    {
        selectionRectangle.x = selectionStartPosition.x;
        selectionRectangle.width = direction.x;
    }
    else // draw from mousePos.x to -direction.x instead, raylib doesn't like negative sizes
    {
        selectionRectangle.x = mousePos.x;
        selectionRectangle.width = -direction.x;
    }
}

void GameScreen::handleLeftMouseButton()
{
    if (hoveringUI) return;

    if (!buildingManager->ghostBuildingExists())
    {
        updateSelectedBuilding();
        return;
    }

    RayCollision collision = raycastToGround();
    if (!collision.hit)
    {
        buildingManager->clearGhostBuilding();
        return;
    }

    if (!buildingManager->canScheduleGhostBuilding()) // can't schedule, do nothing
        return;

    if (player->getState() == RUNNING_TO_BUILD) // already building, just schedule and leave player unchanged
    {
        buildingManager->scheduleGhostBuilding();
        return;
    }

    Vector3 targetPosition = calculateTargetPositionToBuildingFromPlayer(buildingManager->getGhostBuilding());
    std::vector<Vector3> positions = pathfindPositions(player->getPosition(), targetPosition);
    player->setPositions(positions, RUNNING_TO_BUILD);
    buildingManager->scheduleGhostBuilding();
}

void GameScreen::handleRightMouseButton()
{
    if (!player->isSelected())
        return;

    RayCollision collision = raycastToGround();
    if (!collision.hit)
        return;

    if (buildingManager->ghostBuildingExists()) // remove building and just walk the player to the location instead
    {
        buildingManager->clearGhostBuilding();
        return;
    }

    if (player->getState() == RUNNING_TO_BUILD) // player was currently running to build something, clear queue
        buildingManager->clearBuildQueue();

    std::vector<Vector3> positions = pathfindPositions(player->getPosition(), collision.point);
    player->setPositions(positions, RUNNING);
}

RayCollision GameScreen::raycastToGround()
{
    float ground = layer->getHeight();
    const float max = 10000.f;
    // Check mouse collision against a plane spanning from -max to max, with y the same as the ground cubes
    // halfCubeSize is used here since the middle of the ground cube is at y=0
    return GetRayCollisionQuad(
        CameraManager::get().getMouseRay(), // TODO: maybe move these collisions to CameraManager?
        { -max, ground, -max },
        { -max, ground,  max },
        {  max, ground,  max },
        {  max, ground, -max }
    );
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
        printf("Found no valid positions, should probably do something about this later, but not now\n");
        return Vector3Zero();
    }

    return positions[0]; // just grab the first one, don't care which one right now
}

std::vector<Vector3> GameScreen::pathfindPositions(Vector3 start, Vector3 goal)
{
    Vector2i startIndex = layer->worldPositionToIndex(start);
    Vector2i goalIndex = layer->worldPositionToIndex(goal);

    std::list<Vector2i> paths = PathFinding::get().findPath(startIndex, goalIndex, layer->getActualObstacles());
    std::vector<Vector3> positions;

    layer->colorTiles(paths);

    for (Vector2i index: paths)
        positions.push_back(layer->indexToWorldPosition(index));

    return positions;
}
