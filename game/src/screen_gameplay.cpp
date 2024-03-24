#include <iostream>
#include <vector>
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "screens.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static int framesCounter = 0;
static int finishScreen = 0;
Camera3D camera = { 0 };
Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

int gridSizeX = 2;
int gridSizeZ = 2;
float cubeSize = 4.f;
typedef struct {
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    float size = 0.f;
    Color color;
} Cube;

std::vector<std::vector<Cube>> grid;

BoundingBox getCubeBoundingBox(Cube cube)
{
    float halfSize = cube.size/2;
    return (BoundingBox) { Vector3AddValue(cube.position, halfSize), Vector3SubtractValue(cube.position, halfSize) };
}

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    // TODO: Initialize GAMEPLAY screen variables here!
    framesCounter = 0;
    finishScreen = 0;

    for (int x = 0; x < gridSizeX; x++)
    {
        std::vector<Cube> vec = std::vector<Cube>();
        for (int z = 0; z < gridSizeZ; z++)
        {
            Vector3 position = { cubeSize * x, cubeSize * 1, cubeSize * z, };
            std::cout << "position.x: " << position.x << ", position.y: " << position.y << ", position.z: " << position.z << '\n';
            Cube cube = { .position = position, .size = cubeSize, .color = DARKGRAY };
            vec.push_back(cube);
        }
        grid.push_back(vec);
    }

    // std::cout << "gridSizeZ/2 + (gridSizeX/2 * gridSizeZ): " << gridSizeZ/2 + (gridSizeX/2 * gridSizeZ) << ", size: " << grid.size() << '\n';
    Cube middleCube = grid[gridSizeX/2][gridSizeZ/2];
    camera.position = (Vector3){ 35.0f, 36.0f, 35.0f }; // Camera position
    // camera.position = (Vector3){ 65.0f, 36.0f, 65.0f }; // Camera position
    camera.target = middleCube.position;                // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

}

void updateCamera(void)
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

    // std::cout << "cam pos: " << camera.position.x << ", " << camera.position.y << ", " << camera.position.z << '\n';
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    // TODO: Update GAMEPLAY screen variables here!

    // Press enter or tap to change to ENDING screen
    // if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
    // {
    //     finishScreen = 1;
    //     PlaySound(fxCoin);
    // }
    // UpdateCamera(&camera, CAMERA_FREE);

    updateCamera();

    if (IsKeyPressed('Z')) camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };

    if (IsMouseButtonPressed(0))
    {
        printf("GetMousePosition(): %f, %f\n", GetMousePosition().x, GetMousePosition().y);
        Ray ray = GetMouseRay(GetMousePosition(), camera);
        // Iterate through each cube in the grid
        float closestCollisionDistance = 1000000000.f;
        Cube* cubeHit = NULL;
        // int cubeHitX, cubeHitZ;
        for (int x = 0; x < gridSizeX; ++x)
        {
            for (int z = 0; z < gridSizeZ; ++z)
            {
                // Define the vertices of the top quad of the current cube
                // Vector3 pos = grid[x][z].position;
                RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(grid[x][z]));

                // // Check if the ray intersects with the quad
                printf("collision: %f\n", collision.distance);
                if (collision.hit && collision.distance < closestCollisionDistance) {
                    closestCollisionDistance = collision.distance;
                    printf("hit cube with x, z: %d, %d\n", x, z);
                    cubeHit = &grid[x][z];
                }
            }
        }

        printf("cubeHit.color: %d\n", ColorToInt(cubeHit->color));
        if (ColorToInt(cubeHit->color) == ColorToInt(DARKGRAY)) cubeHit->color = RED;
        else cubeHit->color = DARKGRAY;
    }


}

void drawCubeGrid(void)
{
    for (std::vector<Cube> vec: grid)
    {
        for (Cube cube: vec)
        {
            // std::cout << "drawing cube with size " << cube.size << " at position.x: " << cube.position.x << ", position.y: " << cube.position.y << ", position.z: " << cube.position.z << '\n';
            DrawCube(cube.position, cube.size, cube.size, cube.size, cube.color);
            DrawCubeWires(cube.position, cube.size, cube.size, cube.size, GRAY);
        }
    }
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    // TODO: Draw GAMEPLAY screen here!
    // DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), PURPLE);
    // Vector2 pos = { 20, 10 };
    // DrawTextEx(font, "GAMEPLAY SCREEN", pos, font.baseSize*3.0f, 4, MAROON);
    // DrawText("PRESS ENTER or TAP to JUMP to ENDING SCREEN", 130, 220, 20, MAROON);

    BeginMode3D(camera);

    drawCubeGrid();
    // DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
    // DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);

    // DrawGrid(20, 1.0f);

    EndMode3D();
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    // TODO: Unload GAMEPLAY screen variables here!
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}