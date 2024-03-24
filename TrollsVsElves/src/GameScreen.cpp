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

    if (floatingBuilding)
        delete floatingBuilding;
}

void GameScreen::init()
{
    cubeSize = { 4.f, 4.f, 4.f };
    gridSize = (Vector2i){ .x = 32, .y = 32 };
    defaultCubeColor = DARKGRAY;

    Layer* groundLayer = new Layer();
    groundLayer->createGrid(gridSize, cubeSize, DARKGRAY, 0.f);

    layers.push_back(groundLayer);

    buildingSize = { cubeSize.x * 2, cubeSize.y, cubeSize.z * 2 };
    defaultBuildingColor = MAGENTA;
    floatingBuilding = nullptr;

    camera = { 0 };
    camera.position = (Vector3){ 35.0f, 36.0f, 35.0f }; // Camera position
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

    if (floatingBuilding)
        floatingBuilding->draw();

    EndMode3D();
}

void GameScreen::update()
{
    updateCamera();

    if (IsKeyPressed(KEY_B))
        floatingBuilding = new Building(Vector3Zero(), buildingSize, defaultBuildingColor);

    if (floatingBuilding)
        updateFloatingBuilding();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (floatingBuilding)
        {
            buildings.push_back(floatingBuilding);
            floatingBuilding = nullptr;
        }
        else if (!floatingBuilding)
        {
            Building* nearestBuilding = raycastToNearestBuilding();
            if (!nearestBuilding)
            {
                if (selectedBuilding) // remove select
                {
                    selectedBuilding->deselect();
                    selectedBuilding = nullptr;
                }
            }
            else if (nearestBuilding)
            {
                if (!selectedBuilding) // new select
                {
                    selectedBuilding = nearestBuilding;
                    selectedBuilding->select();
                }
                else if (selectedBuilding && selectedBuilding != nearestBuilding) // switch select
                {
                    selectedBuilding->deselect();
                    selectedBuilding = nearestBuilding;
                    selectedBuilding->select();
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
    float maxDistance = 80.f;
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

void GameScreen::updateFloatingBuilding()
{
    float halfCubeSize = cubeSize.y / 2;
    const float max = 10000.f;
    // Check mouse collision against a plane spanning from -max to max, with y the same as the ground cubes
    // halfCubeSize is used here since the middle of the ground cube is at y=0
    RayCollision collision = GetRayCollisionQuad(
        GetMouseRay(GetMousePosition(), camera),
        (Vector3){ -max, halfCubeSize, -max },
        (Vector3){ -max, halfCubeSize,  max },
        (Vector3){  max, halfCubeSize,  max },
        (Vector3){  max, halfCubeSize, -max }
    );

    if (collision.hit)
    {
        Vector3 nearestIncrementedPosition = { // Round to nearest cube multiple
            nearestIncrement(collision.point.x, cubeSize.x) + halfCubeSize,
            layers[0]->getHeight() + cubeSize.y / 2 + buildingSize.y / 2,
            nearestIncrement(collision.point.z, cubeSize.z) + halfCubeSize,
        };

        floatingBuilding->setPosition(nearestIncrementedPosition);
    }
    else
        printf("didnt work\n");

}

Building* GameScreen::raycastToNearestBuilding()
{
    Ray ray = GetMouseRay(GetMousePosition(), camera);
    float closestCollisionDistance = std::numeric_limits<float>::infinity();;
    Building* nearestBuilding = nullptr;

    for (Building* building: buildings)
    {
        Cube* cube = building->getCube();
        RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(*cube));

        if (collision.hit && collision.distance < closestCollisionDistance) {
            closestCollisionDistance = collision.distance;
            nearestBuilding = building;
        }
    }

    return nearestBuilding;
}