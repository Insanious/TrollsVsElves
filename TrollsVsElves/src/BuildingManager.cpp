#include "BuildingManager.h"
#include "Player.h"

BuildingManager::BuildingManager(Vector3 defaultBuildingSize, Color defaultBuildingColor, MapGenerator* mapGenerator)
{
    this->defaultBuildingSize = defaultBuildingSize;
    this->defaultBuildingColor = defaultBuildingColor;
    this->mapGenerator = mapGenerator;

    selectedBuilding = nullptr;
    ghostBuilding = nullptr;
    ghostBuildingIsColliding = false;

    // make sure zeroth level buildings are always available to build
    unlockedActions["castle0"] = 1;
    unlockedActions["rock0"] = 1;
    unlockedActions["hall0"] = 1;
}

BuildingManager::~BuildingManager()
{
    for (int i = 0; i < buildings.size(); i++)
        delete buildings[i];

    clearBuildQueue();

    if (ghostBuilding)
        delete ghostBuilding;
}

void BuildingManager::draw()
{
    for (Building* building: buildings)
        building->draw();

    for (Building* building: buildQueue)
        building->draw();

    if (ghostBuilding)
    {
        Cube& cube = ghostBuilding->cube;
        cube.position = Vector3AddValue(cube.position, 0.1f); // prevents z-fighting that occurs when buildings overlap
        ghostBuilding->draw();
        cube.position = Vector3SubtractValue(cube.position, 0.1f); // revert change
    }
}

void BuildingManager::drawBuildingUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine)
{
    std::vector<ActionNode> children = ActionsManager::get().getActionChildren(selectedBuilding->actionId);

    ActionNode fillerButton = ActionNode("filler", "Filler", "filler", {});
    ActionNode sellButton = ActionNode("sell", "Sell", "sell", {});

    int nrOfFillerButtons = nrOfButtons - children.size();
    for (int i = 0; i < nrOfFillerButtons - 1; i++)
        children.push_back(fillerButton);           // add filler buttons between actual buttons and sell button
    if (nrOfFillerButtons)
        children.push_back(sellButton);             // add sellButton last so its the last button

    ActionNode child;
    bool buttonWasPressed = false;
    for (int i = 0; i < children.size(); i += buttonsPerLine)
    {
        for (int j = 0; j < buttonsPerLine; j++)
        {
            child = children[i+j];

            if (child.id == "filler")
                ImGui::InvisibleButton(child.name.c_str(), buttonSize);
            else
            {
                int colors = canPromoteTo(child.id) ? pushButtonEnabled() : pushButtonDisabled();
                if (ImGui::Button(child.name.c_str(), buttonSize))
                {
                    buttonWasPressed = true;
                    resolveBuildingAction(selectedBuilding, child);
                }

                ImGui::PopStyleColor(colors); // remove pushed colors
            }

            if (j != buttonsPerLine - 1) // apply on all except the last
                ImGui::SameLine();
        }
    }

    if (!buttonWasPressed) // check if any button was clicked using number-key buttons
        for (int i = 0; i < children.size(); i++)
            if (IsKeyPressed((KeyboardKey)int(KEY_ONE) + i))
            {
                resolveBuildingAction(selectedBuilding, children[i]);
                break;
            }
}


void BuildingManager::resolveBuildingAction(Building* building, ActionNode& node)
{
    if (node.id == "filler")
        return;
    else if (node.action == "sell")
        building->sold = true;
    else if (node.action == "recruit") // TODO: later // recruit(building);
        printf("'recruit' action is not implemented\n");
    else if (node.action == "buy") // TODO: later // player->tryBuyItem(Item(node.name));
        printf("'buy' action is not implemented\n");
    else if (node.action == "promote" && canPromoteTo(node.id))
        promote(building, node.id);
}

Building* BuildingManager::raycastToBuilding()
{
    Ray ray = CameraManager::get().getMouseRay();
    float closestCollisionDistance = std::numeric_limits<float>::infinity();
    Building* nearestBuilding = nullptr;

    for (Building* building: buildings)
    {
        RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(building->cube));

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

    for (std::string& id: building->previousActionIds)
        unlockedActions[id]--;
    unlockedActions[building->actionId]--;

    delete building;
}

void BuildingManager::update()
{
    updateBuildings(buildings);

    if (ghostBuilding)
        updateGhostBuilding();

    if (selectedBuilding && selectedBuilding->sold) // delete selectedBuilding and pop from buildings vector
    {
        mapGenerator->removeObstacle(selectedBuilding->cube);
        removeBuilding(selectedBuilding);
        selectedBuilding = nullptr;
    }
}

void BuildingManager::updateBuildings(std::vector<Building*>& buildings)
{
    float dt = GetFrameTime();
    for (Building* building: buildings)
    {
        if (building->buildStage == BuildStage::IN_PROGRESS)
        {
            building->buildTimer += dt;
            float adjusted = building->buildTimer / building->buildTime;
            building->cube.color = lerpColor(building->inProgressColor, building->targetColor, adjusted);

            if (building->buildTimer >= building->buildTime)
                building->finishBuild();
        }
    }
}

Building* BuildingManager::yieldBuildQueue()
{
    assert(buildQueue.size() != 0);

    Building* building = buildQueue.front();
    buildQueue.pop_front();
    building->startBuild();
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
    Cube* cubeHit = mapGenerator->raycastToGround();
    if (!cubeHit)
        return;

    Vector3 cubeSize = mapGenerator->cubeSize;
    float ground = mapGenerator->height;
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

    ghostBuilding->cube.position = final;

    ghostBuildingIsColliding = (isColliding(buildings, ghostBuilding) || isColliding(buildQueue, ghostBuilding));
    ghostBuilding->cube.color = ghostBuildingIsColliding ? RED : ghostBuilding->ghostColor;
}

template<typename Container>
bool BuildingManager::isColliding(const Container& buildings, Building* targetBuilding)
{
    BoundingBox targetBoundingBox = getCubeBoundingBox(targetBuilding->cube, 0.8f);

    for (Building* building: buildings)
        if (CheckCollisionBoxes(targetBoundingBox, getCubeBoundingBox(building->cube)))
            return true;

    return false;
}

void BuildingManager::createDebugBuilding(Vector2i index, BuildingType buildingType)
{
    Building* building = new Building(Cube(defaultBuildingSize), buildingType, nullptr);
    unlockedActions[building->actionId]++;

    Vector3 pos = mapGenerator->indexToWorldPosition(index);
    Vector3 cubeSize = mapGenerator->cubeSize;
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

    building->cube.position = Vector3Add(snapped, offset);
    mapGenerator->addObstacle(building->cube);

    building->scheduleBuild();
    building->startBuild();
    building->finishBuild();
    buildings.push_back(building);
}

void BuildingManager::createNewGhostBuilding(BuildingType buildingType, Player* player)
{
    if (ghostBuilding)
        delete ghostBuilding;

    ghostBuilding = new Building(Cube(defaultBuildingSize), buildingType, player);
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

    unlockedActions[ghostBuilding->actionId]++;
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

void BuildingManager::select(Building* building)
{
    selectedBuilding = building;
    selectedBuilding->select();
}

void BuildingManager::deselect()
{
    if (!selectedBuilding)
        return;

    selectedBuilding->deselect();
    selectedBuilding = nullptr;
}

bool BuildingManager::canScheduleGhostBuilding()
{
    return !ghostBuildingIsColliding;
}

void BuildingManager::recruit(Building* building)
{
    std::vector<Vector2i> neighboringIndices = mapGenerator->getNeighboringIndices(building->cube);
    if (!neighboringIndices.size()) // no valid neighboring tiles
    {
        printf("Found no neighboring tiles, should probably do something about this later\n"); // TODO: later
        return;
    }

    Vector3 neighborPosition = mapGenerator->indexToWorldPosition(neighboringIndices[0]);
    float ground = building->cube.position.y;
    Vector3 pos = { neighborPosition.x, ground, neighborPosition.z };
    Vector3 speed = Vector3Scale(Vector3One(), 30);
    // TEMP: disabled
    // Entity* entity = new Entity(pos, speed, BLACK, WORKER);

    // std::vector<Vector3> positions = mapGenerator->pathfindPositions(entity->getPosition(), building->getRallyPoint().position);
    // entity->setPositions(positions);

    // entities.push_back(entity);
}

bool BuildingManager::canPromoteTo(std::string id)
{
    std::vector<std::string> requirements = ActionsManager::get().requirements[id];
    for (const std::string& id: requirements)
        if (unlockedActions[id] == 0) // defaults to 0 if it doesn't exist
            return false;

    return true;
}

void BuildingManager::promote(Building* building, std::string id)
{
    printf("promote %s to %s\n", building->actionId.c_str(), id.c_str());

    unlockedActions[id]++;
    building->previousActionIds.push_back(building->actionId);
    building->actionId = id;
}
