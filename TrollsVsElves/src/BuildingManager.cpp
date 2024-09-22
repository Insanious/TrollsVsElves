#include "BuildingManager.h"
#include "Player.h"

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
        { SHOP,   new AdvancementTree("shop-dependencies.json") }, // TODO: later, create a proper shop-dependencies file
    };
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

void BuildingManager::drawBuildingUIButtons(Building* building, ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine)
{
    AdvancementNode* advancement = building->getAdvancement();
    std::vector<AdvancementNode*> children = advancement->children;

    AdvancementNode fillerButton = AdvancementNode(IdNode("filler", 0), "filler", nullptr, {});
    AdvancementNode sellButton = AdvancementNode(IdNode("sell", 0), "Sell", nullptr, {});

    int nrOfFillerButtons = nrOfButtons - children.size();
    for (int i = 0; i < nrOfFillerButtons - 1; i++)
        children.push_back(&fillerButton);      // add filler buttons between actual buttons and sell button
    if (nrOfFillerButtons)
        children.push_back(&sellButton);        // add sellButton last so its the last button

    AdvancementNode* child = nullptr;
    bool buttonWasPressed = false;
    for (int i = 0; i < children.size(); i += buttonsPerLine)
    {
        for (int j = 0; j < buttonsPerLine; j++)
        {
            child = children[i+j];

            if (child->name == "filler")
                ImGui::InvisibleButton(child->name.c_str(), buttonSize);
            else
            {
                if (canPromoteTo(child)) // push default colors
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.26f, 0.59f, 0.98f, 0.40f});         // found in imgui_draw.cpp@201
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.26f, 0.59f, 0.98f, 1.00f});  // found in imgui_draw.cpp@202
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.06f, 0.53f, 0.98f, 1.00f});   // found in imgui_draw.cpp@203
                }
                else // push disabled colors
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.83f, 0.32f, 0.32f, 0.4f});
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.83f, 0.32f, 0.32f, 0.7f});
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.83f, 0.32f, 0.32f, 1.00f});
                }

                if (ImGui::Button(child->name.c_str(), buttonSize))
                {
                    buttonWasPressed = true;
                    if (child->id.base == "sell")
                        building->sell();
                    else if (child->id.base == "recruit")
                        recruit(building);
                    else if (child->id.base == "buy")
                        player->tryBuyItem(Item(child->name));
                    else if (canPromoteTo(child))
                        promote(building, child);
                }

                ImGui::PopStyleColor(3); // remove pushed colors
            }

            if (j != buttonsPerLine - 1) // apply on all except the last
                ImGui::SameLine();
        }
    }

    if (!buttonWasPressed)
        checkKeyboardPresses(building, children);
}

void BuildingManager::checkKeyboardPresses(Building* building, std::vector<AdvancementNode*> children)
{
    // check if any key between KEY_ONE -> KEY_ONE + children.size() was pressed
    AdvancementNode* child = nullptr;
    for (int i = 0; i < children.size(); i++)
    {
        child = children[i];
        int keyNr = int(KEY_ONE) + i;
        if (IsKeyPressed((KeyboardKey)keyNr))
        {
            if (child->id.base == "filler")
                return;
            else if (child->id.base == "sell")
                building->sell();
            else if (child->id.base == "recruit")
                recruit(building);
            else if (child->id.base == "buy")
                player->tryBuyItem(Item(child->name));
            else if (canPromoteTo(child))
                promote(building, child);
            break;
        }
    }
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
    updateUnlockedAdvancementOfBase(building->getAdvancement()->id.base);

    delete building;
}

void BuildingManager::update()
{
    for (Building* building: buildings)
        building->update();

    for (Building* building: buildQueue)
        building->update();

    if (ghostBuilding)
        updateGhostBuilding();

    for (Entity* entity: entities)
        entity->update();
}

void BuildingManager::setPlayer(Player* player)
{
    this->player = player;
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

void BuildingManager::createDebugBuilding(Vector2i index, BuildingType buildingType)
{
    Building* building = new Building(Cube(defaultBuildingSize), buildingType);
    promote(building, advancementTrees[buildingType]->getRoot());

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

    building->setPosition(Vector3Add(snapped, offset));
    mapGenerator.addObstacle(building->getCube());

    building->scheduleBuild();
    buildQueue.push_back(building);
    yieldBuildQueue();
}

void BuildingManager::createNewGhostBuilding(BuildingType buildingType)
{
    if (ghostBuilding)
        delete ghostBuilding;

    // advancement of new building starts from the root
    ghostBuilding = new Building(Cube(defaultBuildingSize), buildingType);
    promote(ghostBuilding, advancementTrees[buildingType]->getRoot());

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

void BuildingManager::recruit(Building* building)
{
    MapGenerator& mapGenerator = MapGenerator::get();

    std::vector<Vector2i> neighboringIndices = mapGenerator.getNeighboringIndices(building->getCube());
    if (!neighboringIndices.size()) // no valid neighboring tiles
    {
        printf("Found no neighboring tiles, should probably do something about this later\n"); // TODO: later
        return;
    }

    Vector3 neighborPosition = mapGenerator.indexToWorldPosition(neighboringIndices[0]);
    float ground = building->getCube().position.y;
    Vector3 pos = { neighborPosition.x, ground, neighborPosition.z };
    Vector3 speed = Vector3Scale(Vector3One(), 30);
    Entity* entity = new Entity(pos, speed, BLACK, WORKER);

    std::vector<Vector3> positions = mapGenerator.pathfindPositions(entity->getPosition(), building->getRallyPoint().position);
    entity->setPositions(positions);

    entities.push_back(entity);
}

bool BuildingManager::canPromoteTo(AdvancementNode* promotion)
{
    auto isLocked = [this](IdNode dependency)
    {
        std::string base = dependency.base;
        int stage = dependency.stage;
        return unlockedAdvancements.find(base) == unlockedAdvancements.end() || unlockedAdvancements.at(base) < stage;
    };

    bool hasAnyLockedDependency = std::any_of(promotion->dependencies.begin(), promotion->dependencies.end(), isLocked);
    return !hasAnyLockedDependency;
}

void BuildingManager::promote(Building* building, AdvancementNode* promotion)
{
    std::string base = promotion->id.base;
    int stage = promotion->id.stage;
    if (unlockedAdvancements.find(base) == unlockedAdvancements.end() || unlockedAdvancements.at(base) < stage)
        unlockedAdvancements.insert_or_assign(base, stage);

    building->promote(promotion);
}

void BuildingManager::updateUnlockedAdvancementOfBase(std::string base)
{
    int currentStage = unlockedAdvancements.at(base);

    int highestStage = -1;
    IdNode idNode;
    for (Building* other: buildings)
    {
        idNode = other->getAdvancement()->id;
        if (idNode.base == base && (highestStage -1 || idNode.stage > highestStage))
            highestStage = idNode.stage;
    }

    if (highestStage == -1)
        unlockedAdvancements.erase(unlockedAdvancements.find(base));
    else
        unlockedAdvancements.insert_or_assign(base, highestStage);
}