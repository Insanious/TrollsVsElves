#include "PlayerManager.h"

PlayerManager::PlayerManager(BuildingManager* buildingManager)
{
    this->buildingManager = buildingManager;
    selectedPlayer = nullptr;
}

PlayerManager::~PlayerManager()
{
    for (Player* player: players)
        delete player;
}

void PlayerManager::draw()
{
    for (Player* player: players)
        player->draw();
}

void PlayerManager::update()
{
    for (Player* player: players)
        player->update();

    Building* building = buildingManager->buildQueueFront();
    if (buildingManager->buildQueueFront()) // something is getting built
    {
        Player* player = building->owner;
        if (player->hasReachedDestination())
        {
            Building* building = buildingManager->yieldBuildQueue();
            MapGenerator::get().addObstacle(building->cube);

            building = buildingManager->buildQueueFront();
            if (building) // if more in queue, walk to the next target
            {
                Vector3 pos = calculateTargetPositionToCubeFromPlayer(player, building->cube);
                pathfindPlayerToPosition(player, pos);
            }
        }
    }
}

void PlayerManager::addPlayer(Player* player)
{
    players.push_back(player);
    player->setBuildingManager(buildingManager);
}

void PlayerManager::select(Player* player)
{
    selectedPlayer = player;
    player->select();
}

void PlayerManager::deselect()
{
    if (!selectedPlayer)
        return;

    selectedPlayer->deselect();
    selectedPlayer = nullptr;
}

Vector3 PlayerManager::calculateTargetPositionToCubeFromPlayer(Player* player, Cube cube)
{
    MapGenerator& mapGenerator = MapGenerator::get();
    std::vector<Vector2i> indices = mapGenerator.getNeighboringIndices(cube);
    Vector3 entityPosition = player->getPosition();

    std::vector<Vector3> positions;
    Vector3 position;
    for (Vector2i index: indices)
    {
        position = mapGenerator.indexToWorldPosition(index);
        if (position.x == entityPosition.x && position.z == entityPosition.z)
            return position;

        positions.push_back(position);
    }

    if (positions.empty())
    {
        printf("Found no valid positions, should probably do something about this later\n"); // TODO: later
        return Vector3Zero();
    }

    return positions[0]; // just grab the first one, don't care which one right now
}

bool PlayerManager::checkCollisionCapsulePoint(Capsule capsule, Vector2 point)
{
    CameraManager& cameraManager = CameraManager::get();

    Circle bottom = cameraManager.convertSphereToCircle(capsule.startPos, capsule.radius);
    Circle top = cameraManager.convertSphereToCircle(capsule.endPos, capsule.radius);

    bool collisionBottomCircle = CheckCollisionPointCircle(point, bottom.position, bottom.radius);
    bool collisionTopCircle = CheckCollisionPointCircle(point, top.position, top.radius);

    // TODO: add collision against cylinder bounding box
    return collisionBottomCircle || collisionTopCircle;
}

void PlayerManager::pathfindPlayerToCube(Player* player, Cube cube)
{
    Vector3 targetPosition = calculateTargetPositionToCubeFromPlayer(player, cube);
    std::vector<Vector3> positions = player->playerType == PLAYER_TROLL
        ? MapGenerator::get().pathfindPositionsForTroll(player->getPosition(), targetPosition)
        : MapGenerator::get().pathfindPositionsForElf(player->getPosition(), targetPosition);
    player->setPositions(positions);
}

std::vector<Vector3> PlayerManager::pathfindPlayerToPosition(Player* player, Vector3 position)
{
    std::vector<Vector3> positions = player->playerType == PLAYER_TROLL
        ? MapGenerator::get().pathfindPositionsForTroll(player->getPosition(), position)
        : MapGenerator::get().pathfindPositionsForElf(player->getPosition(), position);
    player->setPositions(positions);

    return positions;
}

Player* PlayerManager::raycastToPlayer()
{
    Vector2 mousePos = GetMousePosition();
    for (Player* player: players)
        if (checkCollisionCapsulePoint(player->capsule, mousePos))
            return player;

    return nullptr;
}

Player* PlayerManager::getPlayerWithNetworkID(RakNet::NetworkID networkID)
{
    for (Player* player: players)
        if (networkID == player->GetNetworkID())
            return player;

    return nullptr;
}