#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "BaseScreen.h"
#include "GameScreen.h"

int main ()
{

    const int screenWidth = 1280;
    const int screenHeight = 800;

    Vector2i screenSize = { 1280, 800 };
    BaseScreen* currentScreen = (BaseScreen*) new GameScreen(screenSize);

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenSize.x, screenSize.y, "Hello Raylib");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    while (!WindowShouldClose())
    {
        currentScreen->update();

        BeginDrawing();
            ClearBackground(RAYWHITE);

            rlImGuiBegin();

            currentScreen->draw();
            DrawFPS(0, 0);

            rlImGuiEnd();
        EndDrawing();
    }

    rlImGuiShutdown();

    // cleanup
    CloseWindow();
    return 0;
}