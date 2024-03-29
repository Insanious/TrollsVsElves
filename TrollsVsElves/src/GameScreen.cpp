#include "GameScreen.h"

GameScreen::GameScreen() {}

GameScreen::~GameScreen()
{
    for (int i = 0; i < layers.size(); i++)
        delete layers[i];

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

    Layer* groundLayer = new Layer();
    groundLayer->createGrid(gridSize, cubeSize, DARKGRAY, 0.f);

    layers.push_back(groundLayer);

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
    Vector3 startPos = { 10.f, cubeSize.y + cubeSize.y/2, 10.f };
    Vector3 endPos = { 10.f, cubeSize.y + 8.f, 10.f };
    float radius = 3.f;
    int slices = 16;
    int rings = 16;
    Color playerColor = BLUE;
    Vector3 playerSpeed = Vector3Scale(Vector3One(), 40);
    player->init(Capsule(startPos, endPos, radius, slices, rings, playerColor), playerSpeed);
    showPlayer = false;


    camera = {
        .position = { 30.0f, 60.0f, 30.0f },
        .target = { 0.f, 0.f, 0.f },
        .up = { 0.0f, 1.0f, 0.0f },
        .fovy = 90.0f,
        .projection = CAMERA_PERSPECTIVE,
    };
}

void GameScreen::draw()
{
    BeginMode3D(camera);

    for (Layer* layer: layers)
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
}

void GameScreen::drawUI()
{
    hoveringUI = false;

    bool drawWindow = !selectedBuilding != !showPlayer; // xor
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
            selectedBuilding->drawUIButtons(windowPadding, buttonPadding);
        else if (showPlayer)
        {
            int buttonsPerLine = 2;
            for (int i = 1; i < BUILDING_TYPE::COUNT; i += buttonsPerLine) // start at 1 since 0 = NONE
            {
                for (int j = 0; j < buttonsPerLine; j++)
                {
                    if ((BUILDING_TYPE)(i+j) == BUILDING_TYPE::COUNT) break; // safeguard if not a multiple of buttonsPerLine

                    BUILDING_TYPE buildingType = (BUILDING_TYPE)(i+j);
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
        delete selectedBuilding;
        selectedBuilding = nullptr;
    }

    if (IsKeyPressed(KEY_B))
    {
        if (selectedBuilding)
        {
            selectedBuilding->deselect();
            selectedBuilding = nullptr;
        }

        showPlayer = !showPlayer;
    }

    if (showPlayer) // Check additional mapping keys
    {
        for (int i = 1; i < BUILDING_TYPE::COUNT; i += 1) // start at 1 since 0 = NONE
        {
            BUILDING_TYPE buildingType = (BUILDING_TYPE)i;
            UIMapping mapping = buildingTypeMappings[buildingType];

            if (IsKeyPressed(mapping.key))
                createNewGhostBuilding(buildingType);
        }
    }

    if (ghostBuilding)
        updateGhostBuilding();

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        handleRightMouseButton();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        handleLeftMouseButton();
}

void GameScreen::updateCamera()
{
    // Pan logic
    // printf("mousePos x, y: %f, %f\n", mousePos.x, mousePos.y);

    float cameraPan = 2.2f;
    // float cameraPan = 0.2f;

    if (IsKeyDown(KEY_A))      CameraMoveRight(&camera, -cameraPan, true);
    else if (IsKeyDown(KEY_D)) CameraMoveRight(&camera, cameraPan, true);

    if (IsKeyDown(KEY_W))      CameraMoveForward(&camera, cameraPan, true);
    else if (IsKeyDown(KEY_S)) CameraMoveForward(&camera, -cameraPan, true);

    // This code enables panning with the mouse at the edges, but its a bit awkward right now
    // int edgeOfScreenMargin = 40;
    // Vector2 mousePos = GetMousePosition();

    // bool mouseIsAtEdgeOfScreenLeft = mousePos.x < edgeOfScreenMargin;
    // bool mouseIsAtEdgeOfScreenRight = screenWidth - mousePos.x < edgeOfScreenMargin;
    // bool mouseIsAtEdgeOfScreenUp = mousePos.y < edgeOfScreenMargin;
    // bool mouseIsAtEdgeOfScreenDown = screenHeight - mousePos.y < edgeOfScreenMargin;

    // if (mouseIsAtEdgeOfScreenLeft || IsKeyDown(KEY_A))
    // {
    //     CameraMoveRight(&camera, -cameraPan, true);
    //     if (mouseIsAtEdgeOfScreenLeft) SetMousePosition(edgeOfScreenMargin, mousePos.y);
    // }
    // else if (mouseIsAtEdgeOfScreenRight || IsKeyDown(KEY_D))
    // {
    //     CameraMoveRight(&camera, cameraPan, true);
    //     if (mouseIsAtEdgeOfScreenRight) SetMousePosition(screenWidth - edgeOfScreenMargin, mousePos.y);
    // }

    // if (mouseIsAtEdgeOfScreenUp || IsKeyDown(KEY_W))
    // {
    //     CameraMoveForward(&camera, cameraPan, true);
    //     if (mouseIsAtEdgeOfScreenUp) SetMousePosition(mousePos.x, edgeOfScreenMargin);
    // }
    // else if (mouseIsAtEdgeOfScreenDown || IsKeyDown(KEY_S))
    // {
    //     CameraMoveForward(&camera, -cameraPan, true);
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
        float distance = Vector3Distance(camera.position, camera.target);

        if (scrollUp)
        {
            if (distance + scrollAmount > maxDistance)
                scroll = maxDistance - distance;
        }
        else {
            if (distance - scrollAmount < minDistance)
                scroll = minDistance - distance;
        }

        CameraMoveToTarget(&camera, scroll);
    }
}

void GameScreen::updateGhostBuilding()
{
    float halfCubeSize = cubeSize.y / 2;
    RayCollision collision = raycastToGround();
    if (collision.hit)
    {
        Vector3 nearestIncrementedPosition = { // Round to nearest cube multiple
            nearestIncrement(collision.point.x, cubeSize.x) + halfCubeSize,
            layers[0]->getHeight() + cubeSize.y / 2 + buildingSize.y / 2,
            nearestIncrement(collision.point.z, cubeSize.z) + halfCubeSize,
        };

        ghostBuilding->setPosition(nearestIncrementedPosition);
        ghostBuildingCollision();
    }
}

void GameScreen::updateSelectedBuilding()
{
    Building* building = raycastToBuilding();

    if (!building && !selectedBuilding) return;

    if (!building && selectedBuilding) // remove select
    {
        selectedBuilding->deselect();
        selectedBuilding = nullptr;

        showPlayer = false;
        return;
    }

    if (!selectedBuilding) // new select
    {
        selectedBuilding = building;
        selectedBuilding->select();

        showPlayer = false;
        return;
    }

    if (selectedBuilding && selectedBuilding != building) // switch select
    {
        selectedBuilding->deselect();
        selectedBuilding = building;
        selectedBuilding->select();

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

        if (buildQueue.size()) // if more in queue, walk to the next target
        {
            building = buildQueue.front();
            Vector3 targetPosition = calculateTargetPositionToBuildingFromPlayer(building);
            player->setTargetPosition(targetPosition);
            player->setState(RUNNING_TO_BUILD);
        }
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

    if (player->getState() != RUNNING_TO_BUILD)
    {
        Vector3 targetPosition = calculateTargetPositionToBuildingFromPlayer(ghostBuilding);
        player->setTargetPosition(targetPosition);
        player->setState(RUNNING_TO_BUILD);
    }

    ghostBuilding->scheduleBuild();
    buildQueue.push_back(ghostBuilding);
    ghostBuilding = nullptr;
}

void GameScreen::handleRightMouseButton()
{
    RayCollision collision = raycastToGround();
    if (!collision.hit) return;

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

    player->setTargetPosition({ collision.point.x, player->getPosition().y, collision.point.z });
}

void GameScreen::createNewGhostBuilding(BUILDING_TYPE buildingType)
{
    if (ghostBuilding)
    {
        delete ghostBuilding;
        ghostBuilding = nullptr;
    }

    ghostBuilding = new Building();
    ghostBuilding->init(Cube(buildingSize), buildingType);
}

Building* GameScreen::raycastToBuilding()
{
    Ray ray = GetMouseRay(GetMousePosition(), camera);
    float closestCollisionDistance = std::numeric_limits<float>::infinity();;
    Building* nearestBuilding = nullptr;

    for (Building* building: buildings)
    {
        Cube cube = building->getCube();
        RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(cube));

        if (collision.hit && collision.distance < closestCollisionDistance) {
            closestCollisionDistance = collision.distance;
            nearestBuilding = building;
        }
    }

    return nearestBuilding;
}

RayCollision GameScreen::raycastToGround()
{
    float halfCubeSize = cubeSize.y / 2;
    const float max = 10000.f;
    // Check mouse collision against a plane spanning from -max to max, with y the same as the ground cubes
    // halfCubeSize is used here since the middle of the ground cube is at y=0
    return GetRayCollisionQuad(
        GetMouseRay(GetMousePosition(), camera),
        { -max, halfCubeSize, -max },
        { -max, halfCubeSize,  max },
        {  max, halfCubeSize,  max },
        {  max, halfCubeSize, -max }
    );
}

Vector3 GameScreen::calculateTargetPositionToBuildingFromPlayer(Building* building)
{
    Vector3 buildingPos = building->getCube().position;
    Vector3 playerPos = player->getPosition();
    Vector3 direction = { buildingPos.x - playerPos.x, 0.f, buildingPos.z - playerPos.z }; // ignore y
    Vector3 normalizedDirection = Vector3Normalize(direction);

    float distance = Vector3Length(direction);
    distance -= getCubeDiagonalLength(building->getCube()) / 2;
    distance -= player->getCapsule().radius;

    Vector3 finalDirection = Vector3Scale(normalizedDirection, distance);
    return Vector3Add(playerPos, finalDirection);
}

void GameScreen::ghostBuildingCollision()
{
    BoundingBox ghostBoundingBox;
    BoundingBox compareBoundingBox;
    isGhostBuildingColliding = false;
    ghostBuilding->getCube().color = { 0, 121, 241, 100 };

    for (int i = 0; i < buildings.size(); i++) //Check collision with existing building
    {
        //Lower scale so that we can placebuilding beside each other beside
        ghostBoundingBox = getCubeBoundingBox(ghostBuilding->getCube(),0.8f);
        compareBoundingBox = getCubeBoundingBox(buildings.at(i)->getCube());

        if (CheckCollisionBoxes(ghostBoundingBox, compareBoundingBox)) {
            isGhostBuildingColliding = true;
            ghostBuilding->getCube().color = RED;
            break;
        }
    }

    for (int i = 0; i < buildQueue.size(); i++) //Check collision with building in the build queue
    {
        ghostBoundingBox = getCubeBoundingBox(ghostBuilding->getCube(), 0.8f);
        compareBoundingBox = getCubeBoundingBox(buildQueue.at(i)->getCube());
        if (CheckCollisionBoxes(ghostBoundingBox, compareBoundingBox)) {
            isGhostBuildingColliding = true;
            ghostBuilding->getCube().color = RED;
            break;
        }
    }
}



