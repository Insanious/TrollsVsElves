#include "BuildingManager.h"

BuildingManager::BuildingManager(Vector3 defaultBuildingSize, Color defaultBuildingColor)
{
    this->defaultBuildingSize = defaultBuildingSize;
    this->defaultBuildingColor = defaultBuildingColor;

    ghostBuilding = nullptr;
    ghostBuildingIsColliding = false;

    advancementTrees = {
        { CASTLE, new AdvancementTree("castle-dependencies.json") },
        { ROCK,   new AdvancementTree("rock-dependencies.json") },
        { HALL,   new AdvancementTree("hall-dependencies.json") },
    };

    promotionSignal.connect(this, &BuildingManager::onPromotion);
}

BuildingManager::~BuildingManager()
{
    for (int i = 0; i < buildings.size(); i++)
        delete buildings[i];

    clearBuildQueue();

    if (ghostBuilding)
        delete ghostBuilding;

    for (int i = 0; i < entities.size(); i++)
        delete entities[i];
}

void BuildingManager::draw()
{
    for (Building* building: buildings)
            building->draw();

    for (Building* building: buildQueue)
        building->draw();

    if (ghostBuilding)
    {
        Cube& cube = ghostBuilding->getCube();
        cube.position = Vector3AddValue(cube.position, 0.1f); // prevents z-fighting that occurs when buildings overlap
        ghostBuilding->draw();
        cube.position = Vector3SubtractValue(cube.position, 0.1f); // revert change
    }

    for (Entity* entity: entities)
        entity->draw();
}

Building* BuildingManager::raycastToBuilding()
{
    Ray ray = CameraManager::get().getMouseRay();
    float closestCollisionDistance = std::numeric_limits<float>::infinity();
    Building* nearestBuilding = nullptr;

    for (Building* building: buildings)
    {
        Cube cube = building->getCube();
        RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(cube));

        if (collision.hit && collision.distance < closestCollisionDistance)
        {
            closestCollisionDistance = collision.distance;
            nearestBuilding = building;
        }
    }

    return nearestBuilding;
}

void BuildingManager::removeBuilding(Building* building)
{
    swapAndPop(buildings, building);
    delete building;
}

void BuildingManager::update()
{
    MapGenerator& mapGenerator = MapGenerator::get();
    for (Building* building: buildings)
    {
        building->update();

        if (building->isRecruiting())
        {
            building->setRecruiting(false);

            std::vector<Vector2i> neighboringIndices = mapGenerator.getNeighboringIndices(building->getCube());
            if (!neighboringIndices.size()) // no valid neighboring tiles
            {
                printf("Found no neighboring tiles, should probably do something about this later\n"); // TODO: later
                continue;
            }

            Vector3 neighborPosition = mapGenerator.indexToWorldPosition(neighboringIndices[0]);
            float height = 3.f;
            float ground = mapGenerator.getCubeSize().y / 2;
            Vector3 startPos = { neighborPosition.x, ground, neighborPosition.z };
            Vector3 endPos = { neighborPosition.x, ground + height, neighborPosition.z };
            float radius = 2.f;
            Vector3 speed = Vector3Scale(Vector3One(), 30);
            Entity* entity = new Entity(startPos, speed, BLACK, WORKER);

            if (building->isSelected())
            {
                std::vector<Vector3> positions = mapGenerator.pathfindPositions(endPos, building->getRallyPoint().position);
                entity->setPositions(positions, RUNNING);
            }

            entities.push_back(entity);
        }
    }
    for (Building* building: buildQueue)
        building->update();

    if (ghostBuilding)
        updateGhostBuilding();

    for (Entity* entity: entities)
        entity->update();
}

Building* BuildingManager::yieldBuildQueue()
{
    assert(buildQueue.size() != 0);

    Building* building = buildQueue.front();
    buildQueue.pop_front();
    building->build();
    buildings.push_back(building);

    return building;
}

Building* BuildingManager::buildQueueFront()
{
    return buildQueue.size() ? buildQueue.front() : nullptr;
}

void BuildingManager::clearBuildQueue()
{
    while(buildQueue.size())
    {
        delete buildQueue.back();
        buildQueue.pop_back();
    }
}

void BuildingManager::updateGhostBuilding()
{

    Cube* cubeHit = MapGenerator::get().raycastToGround();
    if (!cubeHit)
        return;

    Vector3 cubeSize = MapGenerator::get().getCubeSize();
    float ground = MapGenerator::get().getHeight();
    const float max = 10000.f;
    // Check mouse collision against a plane spanning from -max to max, with y the same as the cubeHit y-level
    RayCollision collision = GetRayCollisionQuad(
        CameraManager::get().getMouseRay(),
        { -max, ground + (cubeHit->position.y + cubeSize.y/2), -max },
        { -max, ground + (cubeHit->position.y + cubeSize.y/2),  max },
        {  max, ground + (cubeHit->position.y + cubeSize.y/2),  max },
        {  max, ground + (cubeHit->position.y + cubeSize.y/2), -max }
    );

    assert(collision.hit == true); // sanity check

    // should be edge of cube or middle of cube depending on building size multiple of a cube
    float cubeOffset = (int(defaultBuildingSize.x / cubeSize.x)) % 2 == 0
        ? cubeSize.y / 2
        : cubeSize.y;

    Vector3 snapped = {
        nearestIncrement(collision.point.x + cubeOffset, cubeSize.x),
        cubeHit->position.y + cubeSize.y / 2 + defaultBuildingSize.y / 2, // don't need nearest here, just calculate
        nearestIncrement(collision.point.z + cubeOffset, cubeSize.z)
    };

    Vector3 offset = {
        (cubeSize.x - defaultBuildingSize.x) / 2.0f,
        0.f,
        (cubeSize.z - defaultBuildingSize.z) / 2.0f,
    };
    Vector3 final = Vector3Add(snapped, offset);

    ghostBuilding->setPosition(final);

    ghostBuildingIsColliding = (isColliding(buildings, ghostBuilding) || isColliding(buildQueue, ghostBuilding));
    ghostBuilding->getCube().color = ghostBuildingIsColliding ? RED : ghostBuilding->getGhostColor();
}

template<typename Container>
bool BuildingManager::isColliding(const Container& buildings, Building* targetBuilding)
{
    BoundingBox targetBoundingBox = getCubeBoundingBox(targetBuilding->getCube(), 0.8f);

    for (Building* building: buildings)
        if (CheckCollisionBoxes(targetBoundingBox, getCubeBoundingBox(building->getCube())))
            return true;

    return false;
}

void BuildingManager::createDebugBuilding(Vector2i index)
{
    ghostBuilding = new Building(Cube(defaultBuildingSize), ROCK, nullptr, nullptr);
    MapGenerator& mapGenerator = MapGenerator::get();
    Vector3 pos = mapGenerator.indexToWorldPosition(index);
    Vector3 cubeSize = mapGenerator.getCubeSize();
    Vector3 snapped = {
        nearestIncrement(pos.x, cubeSize.x),
        pos.y, // don't need nearest here
        nearestIncrement(pos.z, cubeSize.z)
    };
    Vector3 offset = {
        (cubeSize.x - defaultBuildingSize.x) / 2.0f,
        0.f,
        (cubeSize.z - defaultBuildingSize.z) / 2.0f,
    };

    ghostBuilding->setPosition(Vector3Add(snapped, offset));
    mapGenerator.addObstacle(ghostBuilding->getCube());

    ghostBuilding->scheduleBuild();
    buildQueue.push_back(ghostBuilding);
    yieldBuildQueue();
    ghostBuilding = nullptr;
}

void BuildingManager::createNewGhostBuilding(BuildingType buildingType)
{
    if (ghostBuilding)
        delete ghostBuilding;

    // advancement of new building starts from the root
    AdvancementNode* advancement = advancementTrees[buildingType]->getRoot();
    ghostBuilding = new Building(Cube(defaultBuildingSize), buildingType, advancement, &promotionSignal);

    updateGhostBuilding(); // sets position correctly
}

void BuildingManager::clearGhostBuilding()
{
    if (ghostBuilding)
    {
        delete ghostBuilding;
        ghostBuilding = nullptr;
    }
}

void BuildingManager::scheduleGhostBuilding()
{
    assert(ghostBuilding != nullptr);

    ghostBuilding->scheduleBuild();
    buildQueue.push_back(ghostBuilding);
    ghostBuilding = nullptr;
}

Building* BuildingManager::getGhostBuilding()
{
    return ghostBuilding;
}

bool BuildingManager::ghostBuildingExists()
{
    return ghostBuilding != nullptr;
}

bool BuildingManager::canScheduleGhostBuilding()
{
    return !ghostBuildingIsColliding;
}

std::vector<Entity*> BuildingManager::getEntities()
{
    return entities;
}

void BuildingManager::updateLockedPromotions()
{
    auto isLocked = [this](std::string dependency) { return unlockedAdvancements.find(dependency) == unlockedAdvancements.end(); };

    std::vector<std::string> locked;
    for (Building* building: buildings)
    {
        locked.clear();
        for (AdvancementNode* promotion: building->getPossiblePromotions())
        {
            // check if any dependency is locked
            bool hasLockedDependency = std::any_of(promotion->dependencies.begin(), promotion->dependencies.end(), isLocked);

            if (hasLockedDependency) // add promotion to locked vector if atleast one of the dependencies are locked
                locked.push_back(promotion->id);
        }

        building->updateLockedPromotions(locked);
    }
}

void BuildingManager::onPromotion(std::string promotion)
{
    unlockedAdvancements.insert(promotion);
    updateLockedPromotions();
}