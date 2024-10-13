#include "MapGenerator.h"

MapGenerator::MapGenerator()
{
    defaultCubeColor = DARKGRAY;
    cubeSize = Vector3Scale(Vector3One(), 4.f);
    height = 0.f;
}

MapGenerator::~MapGenerator() {}

void MapGenerator::draw()
{
    for (Cube& cube: grid)
        drawCube(cube);
}

void MapGenerator::generateFromFile(std::string filename)
{
    Json::Value json = parseJsonFile(filename);

    gridSize = { json["width"].asInt(), json["height"].asInt() };

    grid = std::vector<Cube>(gridSize.y * gridSize.x);
    obstacles = std::vector<std::vector<bool>>(gridSize.y, std::vector<bool>(gridSize.x, false));
    elfObstacles = std::vector<std::vector<bool>>(gridSize.y, std::vector<bool>(gridSize.x, false));
    trollObstacles = std::vector<std::vector<bool>>(gridSize.y/2, std::vector<bool>(gridSize.x/2, false));

    Vector2 halfGridSize = { gridSize.x / 2 * cubeSize.x, gridSize.y / 2 * cubeSize.z };
    float groundHeight = height - cubeSize.y/2;
    float layerHeight = groundHeight;
    float px, py;
    int x, y, index, type;
    for (Json::Value& layer: json["layers"])
    {
        for (y = 0; y < gridSize.y; y++)
        {
            for (x = 0; x < gridSize.x; x++)
            {
                index = twoDimToOneDimIndex({ x, y });
                type = layer["data"][index].asInt();
                if (type == 0) // empty tile
                    continue;

                px = cubeSize.x * x - halfGridSize.x;
                py = cubeSize.y * y - halfGridSize.y;

                grid[index] = Cube({ px, layerHeight, py }, cubeSize, defaultCubeColor);

                if (type == 34) // obstruction tile
                {
                    grid[index].position.y = groundHeight; // put it on the ground so it looks "normal"
                    addObstacle(grid[index]);
                }
            }
        }

        layerHeight += cubeSize.y; // increment height with one cubeSize.y per layer
    }
}

void MapGenerator::recalculateObstacles()
{
    // NOTE: this is needed so the player doesn't walk between the edges of two buildings.
    // NOTE: this would be unnecessary if the buildings were cylinder-shaped instead of cube-shaped
    // check whether two edges are touching and if they are, fill in the gaps
    std::copy(obstacles.begin(), obstacles.end(), elfObstacles.begin());
    Vector2i topLeft;
    Vector2i topRight;
    Vector2i bottomLeft;
    Vector2i bottomRight;
    for (int y = 0; y < gridSize.y - 1; ++y)
    {
        for (int x = 0; x < gridSize.x - 1; ++x)
        {
            topLeft        = { x + 0, y + 0 };
            topRight       = { x + 1, y + 0 };
            bottomLeft     = { x + 0, y + 1 };
            bottomRight    = { x + 1, y + 1 };

            if (elfObstacles[topLeft.y][topLeft.x]               // (0, 0 == true)
                && !elfObstacles[topRight.y][topRight.x]         // (0, 1 == false)
                && !elfObstacles[bottomLeft.y][bottomLeft.x]     // (1, 0 == false)
                && elfObstacles[bottomRight.y][bottomRight.x])   // (1, 1 == true)
            {
                elfObstacles[topRight.y][topRight.x] = true;
                elfObstacles[bottomLeft.y][bottomLeft.x] = true;
            }

            else if (!elfObstacles[topLeft.y][topLeft.x]         // (0, 0 == false)
                && elfObstacles[topRight.y][topRight.x]          // (0, 1 == true)
                && elfObstacles[bottomLeft.y][bottomLeft.x]      // (1, 0 == true)
                && !elfObstacles[bottomRight.y][bottomRight.x])  // (1, 1 == false)
            {
                elfObstacles[topLeft.y][topLeft.x] = true;
                elfObstacles[bottomRight.y][bottomRight.x] = true;
            }
        }
    }
}

void MapGenerator::recalculateTrollObstacles()
{
    // shrink original obstacles map by 2 so it can still be used with pathfinding for troll
    // needed to simulate that the troll is twice as big, and shouldn't be able to walk through 1x1 paths
    for (int y = 0; y < gridSize.y - 1; y += 2)
        for (int x = 0; x < gridSize.x - 1; x += 2)
            trollObstacles[y/2][x/2] = (obstacles[y][x] || obstacles[y+1][x] || obstacles[y][x+1] || obstacles[y+1][x+1]);
}

void MapGenerator::addObstacle(Cube cube)
{
    std::vector<Vector2i> indices = getCubeIndices(cube);
    for (Vector2i index: indices)
        obstacles[index.y][index.x] = true;

    recalculateObstacles();
    recalculateTrollObstacles();
}

void MapGenerator::removeObstacle(Cube cube)
{
    std::vector<Vector2i> indices = getCubeIndices(cube);
    for (Vector2i index: indices)
        obstacles[index.y][index.x] = false;

    recalculateObstacles();
    recalculateTrollObstacles();
}

int MapGenerator::twoDimToOneDimIndex(Vector2i index)
{
    return index.y * gridSize.x + index.x;
}

Vector2i MapGenerator::worldPositionToIndex(Vector3 position)
{
    // shift position by half grid size to get positive values only, and add half cube size to correct for cube origin
    Vector2 adjusted = {
        position.x + (gridSize.x/2 * cubeSize.x) + cubeSize.x/2,
        position.z + (gridSize.y/2 * cubeSize.z) + cubeSize.z/2
    };
    Vector2i truncated = {
        int(adjusted.x / cubeSize.x),
        int(adjusted.y / cubeSize.z)
    };

    return truncated;
}

Vector3 MapGenerator::indexToWorldPosition(Vector2i index)
{
    Cube& cube = grid[twoDimToOneDimIndex(index)];

    Vector2i adjusted = {
        index.x - (gridSize.x/2),
        index.y - (gridSize.y/2),
    };

    return {
        adjusted.x * cubeSize.x,
        cube.position.y + cubeSize.y,
        adjusted.y * cubeSize.z,
    };
}

Vector3 MapGenerator::worldPositionAdjusted(Vector3 position)
{
    Vector2i index = worldPositionToIndex(position);
    return indexToWorldPosition(index);
}

std::vector<Vector2i> MapGenerator::getCubeIndices(Cube& cube)
{
    BoundingBox bb = getCubeBoundingBox(cube);
    Vector2i bottomLeft = worldPositionToIndex(bb.min);
    Vector2i topRight = worldPositionToIndex(bb.max);

    std::vector<Vector2i> indices;
    for (int y = bottomLeft.y; y < topRight.y; ++y)     // exclusive for a reason
        for (int x = bottomLeft.x; x < topRight.x; ++x) // exclusive for a reason
            indices.push_back({ x, y });

    return indices;
}

std::vector<Vector2i> MapGenerator::getNeighboringIndices(std::vector<Vector2i> indices)
{
    static const std::vector<Vector2i> directions = { { -1, 0}, { 1, 0}, { 0, -1}, { 0, 1}, { -1, -1}, { 1, 1}, { -1, 1}, { 1, -1} };

    Vector2i pos;
    std::vector<Vector2i> neighboringIndices;
    for (Vector2i index: indices)
    {
        for (Vector2i direction: directions)
        {
            pos = { index.x + direction.x, index.y + direction.y };

            if (pos.x < 0 || pos.x >= gridSize.x || pos.y < 0 || pos.y >= gridSize.y    // out of bounds
            || (std::find(indices.begin(), indices.end(), pos) != indices.end())        // in indices
            || obstacles[pos.y][pos.x])                                                 // is not traversable
                continue;

            neighboringIndices.push_back(pos);
        }
    }

    return neighboringIndices;
}

std::vector<Vector2i> MapGenerator::getNeighboringIndices(Cube cube)
{
    std::vector<Vector2i> indices = getCubeIndices(cube);
    return getNeighboringIndices(indices);
}

void MapGenerator::colorTiles(std::list<Vector2i> indices)
{
    for (Cube& cube: grid)
        cube.color = defaultCubeColor;
    for (Vector2i index: indices)
        grid[twoDimToOneDimIndex(index)].color = RED;
}

std::vector<Vector3> MapGenerator::pathfindPositionsForElf(Vector3 start, Vector3 goal)
{
    Vector2i startIndex = worldPositionToIndex(start);
    Vector2i goalIndex = worldPositionToIndex(goal);

    std::list<Vector2i> paths = AStar::findPath(startIndex, goalIndex, elfObstacles);
    std::vector<Vector3> positions;

    colorTiles(paths);

    for (Vector2i index: paths)
        positions.push_back(indexToWorldPosition(index));

    return positions;
}

std::vector<Vector3> MapGenerator::pathfindPositionsForTroll(Vector3 start, Vector3 goal)
{
    Vector2i startIndex = worldPositionToIndex(start);
    Vector2i goalIndex = worldPositionToIndex(goal);

    Vector2i startTrollIndex = { startIndex.x/2, startIndex.y/2 }; // half to account for troll obstacle map
    Vector2i goalTrollIndex = { goalIndex.x/2, goalIndex.y/2 }; // half to account for troll obstacle map
    std::list<Vector2i> paths = AStar::findPath(startTrollIndex, goalTrollIndex, trollObstacles);
    std::vector<Vector3> positions;

    Vector3 pos;
    float halfCubeSize = cubeSize.x/2;
    for (Vector2i index: paths)
    {
        pos = indexToWorldPosition({ index.x * 2, index.y * 2 }); // double index to get real index
        positions.push_back({ pos.x + halfCubeSize, pos.y, pos.z + halfCubeSize }); // adjust to make pos middle of 2x2
    }

    std::list<Vector2i> updatedPaths;
    Vector2i doubleIndex;
    for (Vector2i index: paths)
    {
        doubleIndex = { index.x * 2, index.y * 2 }; // top left
        updatedPaths.push_back(doubleIndex);
        updatedPaths.push_back({ doubleIndex.x + 1, doubleIndex.y + 0 }); // top right
        updatedPaths.push_back({ doubleIndex.x + 0, doubleIndex.y + 1 }); // bottom left
        updatedPaths.push_back({ doubleIndex.x + 1, doubleIndex.y + 1 }); // bottom right
    }

    colorTiles(updatedPaths);

    return positions;
}

Cube* MapGenerator::raycastToGround()
{
    Ray ray = CameraManager::get().getMouseRay();
    float closestCollisionDistance = std::numeric_limits<float>::infinity();
    Cube* nearestCube = nullptr;

    for (Cube& cube: grid)
    {
        RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(cube));

        if (collision.hit && collision.distance < closestCollisionDistance)
        {
            closestCollisionDistance = collision.distance;
            nearestCube = &cube;
        }
    }

    return nearestCube;
}
