#include "raylib.h"
#include "BaseScreen.h"
#include "GameScreen.h"

int main ()
{
    const int screenWidth = 1280;
    const int screenHeight = 800;
    BaseScreen* currentScreen = (BaseScreen*) new GameScreen();
    currentScreen->init();

    InitWindow(screenWidth, screenHeight, "Hello Raylib");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        currentScreen->update();

        BeginDrawing();
            ClearBackground(RAYWHITE);

            currentScreen->draw();

        EndDrawing();
    }

    // cleanup
    CloseWindow();
    return 0;
}