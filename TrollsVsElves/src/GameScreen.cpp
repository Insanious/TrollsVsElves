#include "GameScreen.h"
#include "NetworkManager.h"

GameScreen::GameScreen(Vector2i screenSize, bool isSinglePlayer)
{
    this->screenSize = screenSize;
    this->networkManager = nullptr;

    ActionsManager::get().loadRequirements("requirements.json");
    ActionsManager::get().loadActions("actions.json");

    mapGenerator = new MapGenerator();
    mapGenerator->generateFromFile("map/map.json");
    Vector2i gridSize = mapGenerator->gridSize;
    Vector3 cubeSize = mapGenerator->cubeSize;

    buildingManager = new BuildingManager({ cubeSize.x * 2, cubeSize.y, cubeSize.z * 2 }, BLANK, mapGenerator);

    playerManager = new PlayerManager(buildingManager, mapGenerator);
    if (isSinglePlayer)
    {
        Vector3 startPos = { 0.f, cubeSize.y / 2, 0.f };
        startPos.x = gridSize.x / 2 * cubeSize.x - cubeSize.x; // spawn in corner
        startPos.z = gridSize.y / 2 * cubeSize.z - cubeSize.z; // spawn in corner
        Player* player = new Player(startPos, PLAYER_ELF);
        playerManager->addPlayer(player);
        playerManager->clientPlayer = player;
    }

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

        mapGenerator->draw();

        if (buildingManager)
            buildingManager->draw();

        if (playerManager)
            playerManager->draw();

        bool shouldDrawActionWindow = (buildingManager->selectedIndex == -1 != !playerManager->selectedPlayer); // xor
        if (shouldDrawActionWindow) // xor
        {
            size_t nrOfButtons = 4;
            std::vector<ActionNode> actions = buildingManager->selectedIndex != -1
                ? buildingManager->getActions(buildingManager->buildings[buildingManager->selectedIndex], nrOfButtons)
                : playerManager->selectedPlayer->getActions(nrOfButtons);

            UIManager::drawActionButtons(actions, screenSize);
        }

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

RayCollisionObject GameScreen::raycastWorld()
{
    std::variant<std::monostate, Player*, Entity*, Building*, Cube*> variant;
    if (ImGui::GetIO().WantCaptureMouse)
        return RayCollisionObject{ RAYCAST_HIT_TYPE_UI };

    if (Player* player = playerManager->raycastToPlayer())
        return RayCollisionObject{ RAYCAST_HIT_TYPE_PLAYER, (variant = player) };

    if (Building* building = buildingManager->raycastToBuilding())
        return RayCollisionObject{ RAYCAST_HIT_TYPE_BUILDING, (variant = building) };

    if (Cube* cube = mapGenerator->raycastToGround())
        return RayCollisionObject{ RAYCAST_HIT_TYPE_GROUND, (variant = cube) };

    return RayCollisionObject{ RAYCAST_HIT_TYPE_OUT_OF_BOUNDS };
}

void GameScreen::handleLeftMouseButton()
{
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsedTime = now - lastLeftMouseButtonClick;
    bool doubleclicked = elapsedTime.count() < 0.2f;
    lastLeftMouseButtonClick = now;

    RayCollisionObject raycastHit = raycastWorld();
    RaycastHitType type = raycastHit.type;

    // deselect selectedBuilding if not clicking UI
    if (buildingManager->selectedIndex != -1 && type != RAYCAST_HIT_TYPE_UI)
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
            playerManager->select(std::get<Player*>(raycastHit.object));
            break;

        case RAYCAST_HIT_TYPE_BUILDING:
            buildingManager->select(std::get<Building*>(raycastHit.object));
            break;

        case RAYCAST_HIT_TYPE_GROUND:
        {
            if (playerManager->clientPlayer->selected && buildingManager->ghost.exists())
            {
                if (buildingManager->ghost.isColliding) // can't schedule ghostbuilding
                    break;

                Building& ghost = buildingManager->ghost.get();
                bool buildingsInQueue = buildingManager->buildQueueFront() != nullptr;
                buildingManager->scheduleGhostBuilding();
                if (buildingsInQueue) // something is getting built, just schedule and leave player unchanged
                    break;

                Player* player = playerManager->clientPlayer;
                player->reachedDestination = false;
                Vector3 targetPosition = playerManager->calculateTargetPositionToCubeFromPlayer(player, ghost.cube);
                playerManager->pathfindPlayerToPosition(player, targetPosition);
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
    RayCollisionObject raycastHit = raycastWorld();

    switch (raycastHit.type)
    {
        case RAYCAST_HIT_TYPE_GROUND:
        {
            Cube* cube = std::get<Cube*>(raycastHit.object);
            if (playerManager->clientPlayer->selected) // only allow moving client owned player
            {
                buildingManager->ghost.reset();
                buildingManager->clearBuildQueue();

                Vector3 pos = mapGenerator->worldPositionAdjusted(cube->position);
                Player* player = playerManager->clientPlayer;

                playerManager->pathfindPlayerToPosition(player, pos);

                if (networkManager->isClient())
                {
                    networkManager->messageQueue.push(
                        [this, player, pos]() { this->networkManager->sendPlayerRMBRequest(player, pos); }
                    );
                }

                break;
            }

            if (buildingManager->selectedIndex != -1)
            {
                Vector3 adjusted = mapGenerator->worldPositionAdjusted(cube->position);
                buildingManager->buildings[buildingManager->selectedIndex].rallyPoint.position = adjusted;

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
