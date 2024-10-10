#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "BaseScreen.h"
#include "GameScreen.h"
#include "NetworkClient.h"

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"

#include <functional>
#include <string>
#include <iostream>
#include <thread>
#include <atomic>

// Define constants for network communication
const int SERVER_PORT = 60000;
const int MAX_CLIENTS = 4;

NetworkType parseArguments(int argc, char* argv[])
{
    if (argc != 2) {
        printf("needs exactly 1 argument\n");
        exit(0);
    }

    std::string type = std::string(argv[1]);
    if (type == "server") return SERVER;
    if (type == "client") return CLIENT;

    printf("expected either 'server' or 'client' as argument\n");
    exit(0);
};

int main(int argc, char* argv[])
{
    NetworkType type = parseArguments(argc, argv);

    Vector2i screenSize = { 1280, 800 };
    GameScreen* gameScreen = new GameScreen(screenSize);

    NetworkInterface* interface = type == SERVER
        ? (NetworkInterface*)new Server(SERVER_PORT, MAX_CLIENTS, gameScreen)
        : (NetworkInterface*)new Client(SERVER_PORT, gameScreen);

    NetworkClient::get().setNetworkInterface(interface); // Set singleton network client to be used inside game

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenSize.x, screenSize.y, "Hello Raylib");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    std::thread networkThread = std::thread([interface]() { interface->listen(); });

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

    interface->running = false;
    if (networkThread.joinable())
        networkThread.join();

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}