#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "BaseScreen.h"
#include "GameScreen.h"
#include "NetworkManager.h"

#include <functional>
#include <string>
#include <iostream>
#include <thread>
#include <atomic>

// Define constants for network communication
const int SERVER_PORT = 60000;

NetworkType parseNetworkType(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("needs exactly 1 argument\n");
        exit(0);
    }

    std::string type = std::string(argv[1]);
    if (type == "server")   return SERVER;
    if (type == "client")   return CLIENT;
    if (type == "none")     return NONE;

    printf("expected either 'server', 'client' or 'none' as argument\n");
    exit(0);
};

int main(int argc, char* argv[])
{
    NetworkType type = parseNetworkType(argc, argv);
    bool isSinglePlayer = type == NONE;

    Vector2i screenSize = {
        640 * (1 + (int)isSinglePlayer),
        400 * (1 + (int)isSinglePlayer)
    };
    GameScreen* gameScreen = new GameScreen(screenSize, isSinglePlayer);

    NetworkManager networkManager(type, SERVER_PORT, gameScreen);
    gameScreen->networkManager = &networkManager;

    // SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenSize.x, screenSize.y, type == SERVER ? "server" : "client");
    // SetTargetFPS(60);
    rlImGuiSetup(true);

    std::thread networkThread = std::thread([&networkManager]() { networkManager.listen(); });

    while (!WindowShouldClose())
    {
        gameScreen->update();

        BeginDrawing();
            ClearBackground(RAYWHITE);
            rlImGuiBegin();

            gameScreen->draw();
            DrawFPS(0, 0);

            rlImGuiEnd();
        EndDrawing();
    }

    networkManager.running = false;
    if (networkThread.joinable())
        networkThread.join();

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}