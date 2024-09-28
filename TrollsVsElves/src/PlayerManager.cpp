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
            MapGenerator::get().addObstacle(building->getCube());

            building = buildingManager->buildQueueFront();
            if (building) // if more in queue, walk to the next target
                pathfindEntityToCube(player, building->getCube());
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

Vector3 PlayerManager::calculateTargetPositionToCubeFromEntity(Entity* entity, Cube cube)
{
    MapGenerator& mapGenerator = MapGenerator::get();
    std::vector<Vector2i> indices = mapGenerator.getNeighboringIndices(cube);
    Vector3 entityPosition = entity->getPosition();

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
    Vector2 bottomCirclePosScreen = CameraManager::get().getWorldToScreen(capsule.startPos);
    Vector2 topCirclePosScreen = CameraManager::get().getWorldToScreen(capsule.endPos);

    float newBottomRadius = calculateCircleRadius2D(capsule.startPos, capsule.radius);
    float newTopRadius = calculateCircleRadius2D(capsule.endPos, capsule.radius);

    // TODO: add collision against cylinder bounding box
    if (CheckCollisionPointCircle(point, bottomCirclePosScreen, newBottomRadius)  // check against bottom circle
    ||  CheckCollisionPointCircle(point, topCirclePosScreen, newTopRadius))       // check against top circle
        return true;

    return false;
}

float PlayerManager::calculateCircleRadius2D(Vector3 position, float radius)
{
    Matrix viewMatrix = CameraManager::get().getCameraViewMatrix();

    Vector3 right = { viewMatrix.m0, viewMatrix.m1, viewMatrix.m2 };
    Vector3 rightScaled = Vector3Scale(right, radius);
    Vector3 edgeOfCircle = Vector3Add(position, rightScaled);

    return Vector2Distance(
        CameraManager::get().getWorldToScreen(position),
        CameraManager::get().getWorldToScreen(edgeOfCircle)
    );
}

void PlayerManager::pathfindEntityToCube(Entity* entity, Cube cube)
{
    Vector3 targetPosition = calculateTargetPositionToCubeFromEntity(entity, cube);
    std::vector<Vector3> positions = MapGenerator::get().pathfindPositions(entity->getPosition(), targetPosition);
    entity->setPositions(positions);
}

Player* PlayerManager::raycastToPlayer()
{
    Vector2 mousePos = GetMousePosition();
    for (Player* player: players)
        if (checkCollisionCapsulePoint(player->getCapsule(), mousePos))
            return player;

    return nullptr;
}