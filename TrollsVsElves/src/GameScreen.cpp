#include "GameScreen.h"

GameScreen::GameScreen() {}

GameScreen::~GameScreen()
{
    if (layer)
        delete layer;

    for (int i = 0; i < buildings.size(); i++)
        delete buildings[i];

    if (ghostBuilding)
        delete ghostBuilding;

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

    buildingSize = { cubeSize.x * 2, cubeSize.y, cubeSize.z * 2 };
    defaultBuildingColor = WHITE;
    ghostBuilding = nullptr;
    selectedBuilding = nullptr;

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

    camera3D = {
        .position = { 30.0f, 60.0f, 30.0f },
        .target = { 0.f, 0.f, 0.f },
        .up = { 0.0f, 1.0f, 0.0f },
        .fovy = 90.0f,
        .projection = CAMERA_PERSPECTIVE,
    };
    // since we are using CAMERA_PERSPECTIVE, this is static apart from the position (m12, m13, m14)
    cameraViewMatrix = GetCameraMatrix(camera3D);

    camera2D = {
        .offset = { 0.f, 0.f },
        .target = { 0.f, 0.f },
        .rotation = 0.f,
        .zoom = 1.f
    };

    canSelect = true;
    isSelecting = false;
}

void GameScreen::draw()
{
    BeginMode3D(camera3D);

        layer->draw();

        for (Building* building: buildings)
            building->draw();

        for (Building* building: buildQueue)
            building->draw();

        if (ghostBuilding)
            ghostBuilding->draw();

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

                float newBottomRadius = calculateCircleRadius2D(cap.startPos, cap.radius, camera3D);
                float newTopRadius = calculateCircleRadius2D(cap.endPos, cap.radius, camera3D);

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
                        createNewGhostBuilding(buildingType);

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
    updateCamera();

    for (Building* building: buildings)
        building->update();
    for (Building* building: buildQueue)
        building->update();

    player->update();

    if (buildQueue.size()) // something is getting built
        updateBuildQueue();

    if (selectedBuilding && selectedBuilding->isSold()) // delete selectedBuilding and pop from buildings vector
    {
        swapAndPop(buildings, selectedBuilding);
        layer->removeObstacle(selectedBuilding->getCube());
        delete selectedBuilding;
        selectedBuilding = nullptr;
    }

    if (player->isSelected()) // Check additional mapping keys
    {
        for (int i = 1; i < BuildingType::COUNT; i += 1) // start at 1 since 0 = NONE
        {
            BuildingType buildingType = (BuildingType)i;
            UIMapping mapping = buildingTypeMappings[buildingType];

            if (IsKeyPressed(mapping.key))
                createNewGhostBuilding(buildingType);
        }
    }

    if (ghostBuilding)
        updateGhostBuilding();

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        handleRightMouseButton();

    if (!canSelect && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        canSelect = true;

    bool leftMouseButtonWasPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (leftMouseButtonWasPressed)
        handleLeftMouseButton();



    if (canSelect && !hoveringUI && !ghostBuilding)
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

                if (checkCollisionCapsuleRectangle(player->getCapsule(), selectionRectangle, camera3D))
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

void GameScreen::updateCamera()
{
    // Pan logic
    // printf("mousePos x, y: %f, %f\n", mousePos.x, mousePos.y);

    float cameraPan = 2.2f;
    // float cameraPan = 0.2f;

    if (IsKeyDown(KEY_A))      CameraMoveRight(&camera3D, -cameraPan, true);
    else if (IsKeyDown(KEY_D)) CameraMoveRight(&camera3D, cameraPan, true);

    if (IsKeyDown(KEY_W))      CameraMoveForward(&camera3D, cameraPan, true);
    else if (IsKeyDown(KEY_S)) CameraMoveForward(&camera3D, -cameraPan, true);

    // This code enables panning with the mouse at the edges, but its a bit awkward right now
    // int edgeOfScreenMargin = 40;
    // Vector2 mousePos = GetMousePosition();

    // bool mouseIsAtEdgeOfScreenLeft = mousePos.x < edgeOfScreenMargin;
    // bool mouseIsAtEdgeOfScreenRight = screenWidth - mousePos.x < edgeOfScreenMargin;
    // bool mouseIsAtEdgeOfScreenUp = mousePos.y < edgeOfScreenMargin;
    // bool mouseIsAtEdgeOfScreenDown = screenHeight - mousePos.y < edgeOfScreenMargin;

    // if (mouseIsAtEdgeOfScreenLeft || IsKeyDown(KEY_A))
    // {
    //     CameraMoveRight(&camera3D, -cameraPan, true);
    //     if (mouseIsAtEdgeOfScreenLeft) SetMousePosition(edgeOfScreenMargin, mousePos.y);
    // }
    // else if (mouseIsAtEdgeOfScreenRight || IsKeyDown(KEY_D))
    // {
    //     CameraMoveRight(&camera3D, cameraPan, true);
    //     if (mouseIsAtEdgeOfScreenRight) SetMousePosition(screenWidth - edgeOfScreenMargin, mousePos.y);
    // }

    // if (mouseIsAtEdgeOfScreenUp || IsKeyDown(KEY_W))
    // {
    //     CameraMoveForward(&camera3D, cameraPan, true);
    //     if (mouseIsAtEdgeOfScreenUp) SetMousePosition(mousePos.x, edgeOfScreenMargin);
    // }
    // else if (mouseIsAtEdgeOfScreenDown || IsKeyDown(KEY_S))
    // {
    //     CameraMoveForward(&camera3D, -cameraPan, true);
    //     if (mouseIsAtEdgeOfScreenDown) SetMousePosition(mousePos.x, screenHeight - edgeOfScreenMargin);
    // }

    // Zoom logic
    float maxDistance = 120.f;
    float minDistance = 10.f;
    float scrollAmount = 1.f;
    float scroll = -GetMouseWheelMove(); // inverted for a reason
    if (scroll)
    {
        bool scrollUp = scroll == 1;
        float distance = Vector3Distance(camera3D.position, camera3D.target);

        if (scrollUp)
        {
            if (distance + scrollAmount > maxDistance)
                scroll = maxDistance - distance;
        }
        else {
            if (distance - scrollAmount < minDistance)
                scroll = minDistance - distance;
        }

        CameraMoveToTarget(&camera3D, scroll);
    }
}

void GameScreen::updateGhostBuilding()
{
    RayCollision collision = raycastToGround();
    if (!collision.hit)
        return;

    // should be edge of cube or middle of cube depending on building size multiple of a cube
    float cubeOffset = (int(buildingSize.x / cubeSize.x)) % 2 == 0
        ? cubeSize.y / 2
        : cubeSize.y;

    Vector2 snapped = {
        nearestIncrement(collision.point.x + cubeOffset, cubeSize.x),
        nearestIncrement(collision.point.z + cubeOffset, cubeSize.z)
    };
    Vector2 offset = {
        (cubeSize.x - buildingSize.x) / 2.0f,
        (cubeSize.z - buildingSize.z) / 2.0f,
    };
    Vector2 adjusted = Vector2Add(snapped, offset);
    Vector3 final = { adjusted.x, buildingSize.y / 2, adjusted.y };

    ghostBuilding->setPosition(final);

    isGhostBuildingColliding = checkBuildingCollisionsAgainstTarget(buildings, ghostBuilding)
        || checkBuildingCollisionsAgainstTarget(buildQueue, ghostBuilding);

    ghostBuilding->getCube().color = isGhostBuildingColliding ? RED : ghostBuilding->getGhostColor();
}

void GameScreen::updateSelectedBuilding()
{
    Building* building = raycastToBuilding();

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

void GameScreen::updateBuildQueue()
{
    if (player->getState() == IDLE && player->getPreviousState() == RUNNING_TO_BUILD) // was running and reached target
    {
        Building* building = buildQueue.front();
        buildQueue.pop_front();
        building->build();
        buildings.push_back(building);

        layer->addObstacle(building->getCube());

        if (buildQueue.size()) // if more in queue, walk to the next target
        {
            building = buildQueue.front();
            Vector3 targetPosition = calculateTargetPositionToBuildingFromPlayer(building);
            std::vector<Vector3> positions = pathfindPositions(player->getPosition(), targetPosition);
            player->setPositions(positions, RUNNING_TO_BUILD);
        }
    }
}

float GameScreen::calculateCircleRadius2D(Vector3 position, float radius, Camera3D camera)
{
    Vector3 right = { cameraViewMatrix.m0, cameraViewMatrix.m1, cameraViewMatrix.m2 };
    Vector3 rightScaled = Vector3Scale(right, radius);
    Vector3 edgeOfCircle = Vector3Add(position, rightScaled);

    return Vector2Distance(
        GetWorldToScreen(position, camera),
        GetWorldToScreen(edgeOfCircle, camera)
    );
}

bool GameScreen::checkCollisionCapsuleRectangle(Capsule capsule, Rectangle rectangle, Camera3D camera)
{
    Vector2 bottomCirclePosScreen = GetWorldToScreen(capsule.startPos, camera3D);
    Vector2 topCirclePosScreen = GetWorldToScreen(capsule.endPos, camera3D);

    float newBottomRadius = calculateCircleRadius2D(capsule.startPos, capsule.radius, camera3D);
    float newTopRadius = calculateCircleRadius2D(capsule.endPos, capsule.radius, camera3D);

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

    if (!ghostBuilding)
    {
        updateSelectedBuilding();
        return;
    }

    RayCollision collision = raycastToGround();
    if (!collision.hit)
    {
        delete ghostBuilding;
        ghostBuilding = nullptr;

        return;
    }

    if (isGhostBuildingColliding) return;

    if (player->getState() == RUNNING_TO_BUILD)
    {
        ghostBuilding->scheduleBuild();
        buildQueue.push_back(ghostBuilding);
        ghostBuilding = nullptr;

        return;
    }

    Vector3 targetPosition = calculateTargetPositionToBuildingFromPlayer(ghostBuilding);
    std::vector<Vector3> positions = pathfindPositions(player->getPosition(), targetPosition);
    player->setPositions(positions, RUNNING_TO_BUILD);

    ghostBuilding->scheduleBuild();
    buildQueue.push_back(ghostBuilding);
    ghostBuilding = nullptr;
}

void GameScreen::handleRightMouseButton()
{
    if (!player->isSelected())
        return;

    RayCollision collision = raycastToGround();
    if (!collision.hit)
        return;

    if (ghostBuilding) // remove building and just walk the player to the location instead
    {
        delete ghostBuilding;
        ghostBuilding = nullptr;
        return;
    }

    if (player->getState() == RUNNING_TO_BUILD) // player was currently running to build something
    {
        while(buildQueue.size()) // clear the whole buildQueue
        {
            delete buildQueue.back();
            buildQueue.pop_back();
        }
    }

    std::vector<Vector3> positions = pathfindPositions(player->getPosition(), collision.point);
    player->setPositions(positions, RUNNING);
}

void GameScreen::createNewGhostBuilding(BuildingType buildingType)
{
    if (ghostBuilding)
    {
        delete ghostBuilding;
        ghostBuilding = nullptr;
    }

    ghostBuilding = new Building();
    ghostBuilding->init(Cube(buildingSize), buildingType);

    canSelect = false;
}

Building* GameScreen::raycastToBuilding()
{
    Ray ray = GetMouseRay(GetMousePosition(), camera3D);
    float closestCollisionDistance = std::numeric_limits<float>::infinity();;
    Building* nearestBuilding = nullptr;

    for (Building* building: buildings)
    {
        Cube cube = building->getCube();
        RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(cube));

        if (collision.hit && collision.distance < closestCollisionDistance)
        {
            closestCollisionDistance = collision.distance;
            nearestBuilding = building;
        }
    }

    return nearestBuilding;
}

RayCollision GameScreen::raycastToGround()
{
    float ground = layer->getHeight();
    const float max = 10000.f;
    // Check mouse collision against a plane spanning from -max to max, with y the same as the ground cubes
    // halfCubeSize is used here since the middle of the ground cube is at y=0
    return GetRayCollisionQuad(
        GetMouseRay(GetMousePosition(), camera3D),
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

template<typename Container>
bool GameScreen::checkBuildingCollisionsAgainstTarget(const Container& buildings, Building* targetBuilding)
{
    BoundingBox targetBoundingBox = getCubeBoundingBox(targetBuilding->getCube(), 0.8f);

    for (Building* building: buildings)
        if (CheckCollisionBoxes(targetBoundingBox, getCubeBoundingBox(building->getCube())))
            return true;

    return false;
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
