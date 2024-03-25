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
    BaseScreen* currentScreen = (BaseScreen*) new GameScreen();
    currentScreen->init(screenSize);

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

            // bool open = true;
            // int window_flags = 0;
            // ImGui::ShowDemoWindow(&open);

            // // Main body of the Demo window starts here.
            // if (!ImGui::Begin("Dear ImGui Demo", &open, window_flags))
            // {
            //     // Early out if the window is collapsed, as an optimization.
            //     ImGui::End();
            //     return 1;
            // }

            // Setup Dear ImGui context
            // IMGUI_CHECKVERSION();
            // ImGuiIO& io = ImGui::GetIO(); (void)io;
            // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

            // {
            //     static float f = 0.0f;
            //     static int counter = 0;
            //     ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

            //     ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            //     ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            //     ImGui::Checkbox("Demo Window", &open);      // Edit bools storing our window open/close state

            //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            //     ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            //     if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //         counter++;
            //     ImGui::SameLine();
            //     ImGui::Text("counter = %d", counter);

            //     // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            //     ImGui::End();
            // }
            currentScreen->draw();

            rlImGuiEnd();
        EndDrawing();
    }

    rlImGuiShutdown();

    // cleanup
    CloseWindow();
    return 0;
}