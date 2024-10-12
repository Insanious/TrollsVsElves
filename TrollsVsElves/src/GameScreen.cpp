#include "GameScreen.h"
#include "NetworkClient.h"

GameScreen::GameScreen(Vector2i screenSize)
{
    this->screenSize = screenSize;

    ActionsManager::get().loadRequirements("requirements.json");
    ActionsManager::get().loadActions("actions.json");

    MapGenerator::get().generateFromFile("map/map.json");
    Vector2i gridSize = MapGenerator::get().getGridSize();
    Vector3 cubeSize = MapGenerator::get().getCubeSize();

    buildingManager = new BuildingManager({ cubeSize.x * 2, cubeSize.y, cubeSize.z * 2 }, BLANK);

    Vector3 startPos = { 0.f, cubeSize.y / 2, 0.f };
    startPos.x = gridSize.x / 2 * cubeSize.x - cubeSize.x; // spawn in corner
    startPos.z = gridSize.y / 2 * cubeSize.z - cubeSize.z; // spawn in corner

    playerManager = new PlayerManager(buildingManager);
    playerManager->addPlayer(new Player(startPos, PLAYER_ELF));

    isMultiSelecting = false;

    lastLeftMouseButtonClick = std::chrono::steady_clock::now();

    buildingManager->createDebugBuilding({ 15, 15 }, ROCK);
}

GameScreen::~GameScreen()
{
    if (buildingManager)
        delete buildingManager;

    if (playerManager)
        delete playerManager;
}

void GameScreen::draw()
{
    CameraManager& cameraManager = CameraManager::get();

    BeginMode3D(cameraManager.getCamera());

        MapGenerator::get().draw();

        if (buildingManager)
            buildingManager->draw();

        if (playerManager)
            playerManager->draw();

        drawUI();

    EndMode3D();

    BeginMode2D(cameraManager.getCamera2D());

        if (isMultiSelecting)
        {
            DrawRectangleRec(multiSelectionRectangle, { 0, 255, 0, 25 });
            DrawRectangleLinesEx(multiSelectionRectangle, 1.f, { 0, 255, 0, 50 });

            std::vector<Player*>& players = playerManager->players;

            if (true) // set to true to draw all entities' capsule collision circles
            {
                for (Player* player: players)
                {
                    Capsule& capsule = player->capsule;
                    Circle bottom = cameraManager.convertSphereToCircle(capsule.startPos, capsule.radius);
                    Circle top = cameraManager.convertSphereToCircle(capsule.endPos, capsule.radius);

                    DrawCircleV(bottom.position, bottom.radius, YELLOW);
                    DrawCircleLinesV(bottom.position, bottom.radius, GRAY);

                    DrawCircleV(top.position, top.radius, YELLOW);
                    DrawCircleLinesV(top.position, top.radius, GRAY);
                }
            }
        }

    EndMode2D();
}

void GameScreen::drawUI()
{
    bool drawWindow = (!buildingManager->selectedBuilding != !playerManager->selectedPlayer); // xor
    if (drawWindow)
    {
        bool bottomRightWindow = true;
        int bottomRightWindowFlags = 0;
        bottomRightWindowFlags |= ImGuiWindowFlags_NoTitleBar;
        bottomRightWindowFlags |= ImGuiWindowFlags_NoResize;
        bottomRightWindowFlags |= ImGuiWindowFlags_NoMove;

        ImVec2 windowSize(300, 200);
        ImVec2 windowPos(
            screenSize.x - windowSize.x,
            screenSize.y - windowSize.y
        );

        ImVec2 windowPadding(8, 8);
        ImVec2 buttonPadding(8, 8);
        Vector2i buttonLayout = { 2, 2 }; // 2 columns, 2 rows
        int nrOfButtons = buttonLayout.x * buttonLayout.y;
        ImVec2 buttonSize(
            (windowSize.x / float(buttonLayout.x)) - windowPadding.x - (buttonPadding.x / float(buttonLayout.x)),
            (windowSize.y / float(buttonLayout.y)) - windowPadding.y - (buttonPadding.y / float(buttonLayout.y))
        );

        // Draw bottom right window
        ImGui::SetNextWindowSize(windowSize);
        ImGui::SetNextWindowPos(windowPos);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);
        ImGui::Begin("Bottom right window", &bottomRightWindow, bottomRightWindowFlags);
        ImGui::PopStyleVar();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, buttonPadding);

        if (buildingManager->selectedBuilding)
            buildingManager->drawBuildingUIButtons(buttonSize, nrOfButtons, buttonLayout.x);
        else if (playerManager->selectedPlayer)
            playerManager->selectedPlayer->drawUIButtons(buttonSize, nrOfButtons, buttonLayout.x);

        ImGui::PopStyleVar();
        ImGui::End();
    }

}

void GameScreen::update()
{
    Task task;
    while (messageQueue.pop(task))
        task();

    CameraManager::get().update();
    buildingManager->update();
    playerManager->update();

    if (!isMultiSelecting)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) // LMB was clicked this frame
            handleLeftMouseButton();
    }
    else
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            updateMultiSelection();
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            stopMultiSelection();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        handleRightMouseButton();
}

void GameScreen::startMultiSelection()
{
    isMultiSelecting = true;
    multiSelectionStartPosition = GetMousePosition();
    updateMultiSelection();
}

void GameScreen::stopMultiSelection()
{
    isMultiSelecting = false;

    CameraManager& cameraManager = CameraManager::get();
    std::vector<Player*> players = playerManager->players;

    for (Player* player: players)
    {
        Capsule& capsule = player->capsule;
        Circle bottom = cameraManager.convertSphereToCircle(capsule.startPos, capsule.radius);
        Circle top = cameraManager.convertSphereToCircle(capsule.endPos, capsule.radius);

        // TODO: add collision against cylinder bounding box lines
        if (CheckCollisionCircleRec(bottom.position, bottom.radius, multiSelectionRectangle)
        ||  CheckCollisionCircleRec(top.position, top.radius, multiSelectionRectangle))
        {
            playerManager->deselect();
            playerManager->select(player);
        }
    }
}

void GameScreen::updateMultiSelection()
{
    Vector2 mousePos = GetMousePosition();
    Vector2 direction = Vector2Subtract(mousePos, multiSelectionStartPosition);

    if (direction.y >= 0) // draw from multiSelectionStartPosition.y to direction.y
    {
        multiSelectionRectangle.y = multiSelectionStartPosition.y;
        multiSelectionRectangle.height = direction.y;
    }
    else // draw from mousePos.y to -direction.y instead, raylib doesn't like negative sizes
    {
        multiSelectionRectangle.y = mousePos.y;
        multiSelectionRectangle.height = -direction.y;
    }

    if (direction.x >= 0) // draw from multiSelectionStartPosition.x to direction.x
    {
        multiSelectionRectangle.x = multiSelectionStartPosition.x;
        multiSelectionRectangle.width = direction.x;
    }
    else // draw from mousePos.x to -direction.x instead, raylib doesn't like negative sizes
    {
        multiSelectionRectangle.x = mousePos.x;
        multiSelectionRectangle.width = -direction.x;
    }
}

RaycastHitType GameScreen::checkRaycastHitType()
{
    if (ImGui::GetIO().WantCaptureMouse)
        return RAYCAST_HIT_TYPE_UI;

    if (playerManager->raycastToPlayer())
        return RAYCAST_HIT_TYPE_PLAYER;

    if (buildingManager->raycastToBuilding())
        return RAYCAST_HIT_TYPE_BUILDING;

    if (MapGenerator::get().raycastToGround())
        return RAYCAST_HIT_TYPE_GROUND;

    return RAYCAST_HIT_TYPE_OUT_OF_BOUNDS;
}

void GameScreen::handleLeftMouseButton()
{
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsedTime = now - lastLeftMouseButtonClick;
    bool doubleclicked = elapsedTime.count() < 0.2f;
    lastLeftMouseButtonClick = now;

    RaycastHitType type = checkRaycastHitType();

    // deselect selectedBuilding if not clicking UI
    if (buildingManager->selectedBuilding && type != RAYCAST_HIT_TYPE_UI)
        buildingManager->deselect();

    // always clear selectedEntities if clicked player/entity/building
    if (playerManager->selectedPlayer && type != RAYCAST_HIT_TYPE_UI && type != RAYCAST_HIT_TYPE_GROUND)
        playerManager->deselect();

    switch (type)
    {
        case RAYCAST_HIT_TYPE_UI:
            break;

        case RAYCAST_HIT_TYPE_OUT_OF_BOUNDS:
            startMultiSelection();
            break;

        case RAYCAST_HIT_TYPE_PLAYER:
            playerManager->select(playerManager->raycastToPlayer());
            break;

        case RAYCAST_HIT_TYPE_BUILDING:
        {
            buildingManager->select(buildingManager->raycastToBuilding());
            break;
        }

        case RAYCAST_HIT_TYPE_GROUND:
        {
            if (playerManager->selectedPlayer && buildingManager->ghostBuildingExists())
            {
                if (!buildingManager->canScheduleGhostBuilding()) // can't schedule ghostbuilding
                    break;

                Building* ghost = buildingManager->getGhostBuilding();
                bool buildingsInQueue = buildingManager->buildQueueFront() != nullptr;
                buildingManager->scheduleGhostBuilding();
                if (buildingsInQueue) // something is getting built, just schedule and leave player unchanged
                    break;

                playerManager->selectedPlayer->reachedDestination = false;
                playerManager->pathfindPlayerToCube(playerManager->selectedPlayer, ghost->cube);
                break;
            }

            if (playerManager->selectedPlayer) // a player is not trying to place a ghostbuilding
                playerManager->deselect();

            if (!isMultiSelecting)
                startMultiSelection();

            break;
        }
    }
}

void GameScreen::handleRightMouseButton()
{
    MapGenerator& mapGenerator = MapGenerator::get();
    NetworkClient& networkClient = NetworkClient::get();
    RaycastHitType type = checkRaycastHitType();

    switch (type)
    {
        case RAYCAST_HIT_TYPE_GROUND:
        {
            if (playerManager->selectedPlayer)
            {
                buildingManager->clearGhostBuilding();
                buildingManager->clearBuildQueue();

                Vector3 pos = mapGenerator.worldPositionAdjusted(mapGenerator.raycastToGround()->position);
                Player* player = playerManager->selectedPlayer;

                playerManager->pathfindPlayerToPosition(player, pos);

                if (networkClient.isClient())
                {
                    Client* client = networkClient.getClient();
                    client->messageQueue.push([client, player, pos]() { client->sendPlayerRMBRequest(player, pos); });
                }

                break;
            }

            if (buildingManager->selectedBuilding)
            {
                Vector3 adjusted = mapGenerator.worldPositionAdjusted(raycastToGround().point);
                buildingManager->selectedBuilding->rallyPoint.position = adjusted; // TODO: later, this doesn't work for y-elevated buildings

                break;
            }

            break;
        }

        case RAYCAST_HIT_TYPE_BUILDING:         // TEMP: do nothing
        case RAYCAST_HIT_TYPE_UI:               // do nothing
        case RAYCAST_HIT_TYPE_PLAYER:           // do nothing
        case RAYCAST_HIT_TYPE_OUT_OF_BOUNDS:    // do nothing
            break;
    }
}

RayCollision GameScreen::raycastToGround()
{
    float ground = MapGenerator::get().getHeight();
    const float max = 10000.f;
    // Check mouse collision against a plane spanning from -max to max, with y the same as the ground level
    return GetRayCollisionQuad(
        CameraManager::get().getMouseRay(),
        { -max, ground, -max },
        { -max, ground,  max },
        {  max, ground,  max },
        {  max, ground, -max }
    );
}
