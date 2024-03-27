#include "GameScreen.h"

GameScreen::GameScreen()
{
    printf("GameScreen()\n");
}

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
    buildingUI.setScreenSize(screenSize);

    cubeSize = { 4.f, 4.f, 4.f };
    gridSize = (Vector2i){ .x = 32, .y = 32 };
    defaultCubeColor = DARKGRAY;

    Layer* groundLayer = new Layer();
    groundLayer->createGrid(gridSize, cubeSize, DARKGRAY, 0.f);

    layers.push_back(groundLayer);

    buildingSize = { cubeSize.x * 2, cubeSize.y, cubeSize.z * 2 };
    defaultBuildingColor = WHITE;
    ghostBuilding = nullptr;
    selectedBuilding = nullptr;

    player = new Player();
    Vector3 startPos = { 10.f, cubeSize.y + cubeSize.y/2, 10.f };
    Vector3 endPos = { 10.f, cubeSize.y + 8.f, 10.f };
    float radius = 3.f;
    int slices = 16;
    int rings = 16;
    Color playerColor = BLUE;
    Vector3 playerSpeed = Vector3Scale(Vector3One(), 40);
    player->init(Capsule(startPos, endPos, radius, slices, rings, playerColor), playerSpeed);

    camera = { 0 };
    camera.position = (Vector3){ 30.0f, 60.0f, 30.0f }; // Camera position
    camera.target = (Vector3){ 0.f, 0.f, 0.f };         // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 90.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
}

void GameScreen::draw()
{
    BeginMode3D(camera);

    for (Layer* layer: layers)
        layer->draw();

    for (Building* building: buildings)
        building->draw();

    for (Building* building: buildQueue)
    {
        Color oldColor = building->getCube().color;
        building->getCube().color = RED;
        building->draw();
        building->getCube().color = oldColor;
    }

    if (ghostBuilding)
        ghostBuilding->draw();

    buildingUI.draw();

    if (player)
        player->draw();

    EndMode3D();
}

void GameScreen::update()
{
    updateCamera();

    if (player->isIdle() && buildQueue.size())
    {
        Building* building = buildQueue.front();
        printf("building1: %p\n", building);
        buildQueue.pop_front();
        building->build();
        buildings.push_back(building);

        printf("buildQueue.size(): %d\n", buildQueue.size());
        if (buildQueue.size()) // if more in queue, walk to next
        {
            building = buildQueue.front();
            printf("building2: %p\n", building);
            Vector3 targetPosition = calculateTargetPositionToBuildingFromPlayer(building);
            player->setTargetPosition(targetPosition);
        }
    }

    player->update();

    if (selectedBuilding && selectedBuilding->isSold()) // delete selectedBuilding and pop from buildings vector
        {
            int index = 0;
            for (int i = 0; i < buildings.size(); i++)
                if (buildings[i] == selectedBuilding)
                {
                    index = i;
                    break;
                }

            if (buildings[index] != buildings.back())
                std::swap(buildings[index], buildings.back());

            buildings.pop_back();
            buildings.shrink_to_fit();

            delete selectedBuilding;
            selectedBuilding = nullptr;
            buildingUI.hide();
        }

    if (IsKeyPressed(KEY_B))
        ghostBuilding = new Building(Vector3Zero(), buildingSize, defaultBuildingColor);

    if (ghostBuilding)
        updateGhostBuilding();

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        RayCollision collision = raycastToGround();
        if (collision.hit)
        {
            if (ghostBuilding) // remove building and just walk the player to the location instead
            {
                delete ghostBuilding;
                ghostBuilding = nullptr;
            }
            player->setTargetPosition({ collision.point.x, player->getPosition().y, collision.point.z });
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (buildingUI.isHovering()) { /* do nothing... */ }
        else if (ghostBuilding)
        {
            RayCollision collision = raycastToGround();
            if (collision.hit)
            {
                if (player->isIdle())
                {
                    Vector3 targetPosition = calculateTargetPositionToBuildingFromPlayer(ghostBuilding);
                    player->setTargetPosition(targetPosition);
                }
                buildQueue.push_back(ghostBuilding);
            }
            else
                delete ghostBuilding;

            ghostBuilding = nullptr;
        }
        else if (!ghostBuilding)
        {
            updateSelectedBuilding();
        }
    }
}

void GameScreen::updateCamera()
{
    // Pan logic
    // printf("mousePos x, y: %f, %f\n", mousePos.x, mousePos.y);

    float cameraPan = 2.2f;
    // float cameraPan = 0.2f;

    if (IsKeyDown(KEY_A))
    {
        CameraMoveRight(&camera, -cameraPan, true);
    }
    else if (IsKeyDown(KEY_D))
    {
        CameraMoveRight(&camera, cameraPan, true);
    }

    if (IsKeyDown(KEY_W))
    {
        CameraMoveForward(&camera, cameraPan, true);
    }
    else if (IsKeyDown(KEY_S))
    {
        CameraMoveForward(&camera, -cameraPan, true);
    }

    // This code enables panning with the mouse at the edges
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

    // Scroll logic
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
    const float max = 10000.f;
    // Check mouse collision against a plane spanning from -max to max, with y the same as the ground cubes
    // halfCubeSize is used here since the middle of the ground cube is at y=0
    RayCollision collision = raycastToGround();
    if (collision.hit)
    {
        Vector3 nearestIncrementedPosition = { // Round to nearest cube multiple
            nearestIncrement(collision.point.x, cubeSize.x) + halfCubeSize,
            layers[0]->getHeight() + cubeSize.y / 2 + buildingSize.y / 2,
            nearestIncrement(collision.point.z, cubeSize.z) + halfCubeSize,
        };

        ghostBuilding->setPosition(nearestIncrementedPosition);
    }
}

void GameScreen::updateSelectedBuilding()
{
    Building* nearestBuilding = raycastToNearestBuilding();

    if (!nearestBuilding && selectedBuilding) // remove select
    {
        printf("!nearestBuilding && selectedBuilding\n");
        selectedBuilding->deselect();
        selectedBuilding = nullptr;

        buildingUI.hide();
        return;
    }

    if (nearestBuilding)
    {
        printf("nearestBuilding\n");
        if (!selectedBuilding) // new select
        {
            printf("nearestBuilding && !selectedBuilding\n");
            selectedBuilding = nearestBuilding;
            selectedBuilding->select();

            buildingUI.init(selectedBuilding);
            buildingUI.show();
            return;
        }

        if (selectedBuilding && selectedBuilding != nearestBuilding) // switch select
        {
            printf("nearestBuilding && selectedBuilding && selectedBuilding != nearestBuilding\n");
            selectedBuilding->deselect();
            selectedBuilding = nearestBuilding;
            selectedBuilding->select();

            buildingUI.init(selectedBuilding);
            buildingUI.show();
            return;
        }
    }
}

Building* GameScreen::raycastToNearestBuilding()
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
        (Vector3){ -max, halfCubeSize, -max },
        (Vector3){ -max, halfCubeSize,  max },
        (Vector3){  max, halfCubeSize,  max },
        (Vector3){  max, halfCubeSize, -max }
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
    printf("calculateTargetPositionToBuildingFromPlayer distance: %f\n", distance);

    Vector3 finalDirection = Vector3Scale(normalizedDirection, distance);
    return Vector3Add(playerPos, finalDirection);
}
