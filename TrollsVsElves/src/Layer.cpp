#include "Layer.h"

Layer::Layer()
{
    this->defaultCubeColor = DARKGRAY;
    this->cubeSize = Vector3Scale(Vector3One(), 4.f);
    this->height = 0.f;
}

Layer::~Layer()
{
    for (int i = 0; i < grid.size(); i++)
        delete grid[i];
}

void Layer::draw()
{
    for (Cube* cube: grid)
        drawCube(*cube);
}

void Layer::createFromFile(std::string filename)
{
    Json::Value json = parseJsonFile(filename);

    this->gridSize = {
        json["width"].asInt(),
        json["height"].asInt()
    };

    this->grid = std::vector<Cube*>(gridSize.y * gridSize.x, nullptr);
    obstacles = std::vector<std::vector<bool>>(gridSize.y, std::vector<bool>(gridSize.x, false));
    actualObstacles = std::vector<std::vector<bool>>(gridSize.y, std::vector<bool>(gridSize.x, false));
    trollObstacles = std::vector<std::vector<bool>>(gridSize.y/2, std::vector<bool>(gridSize.x/2, false));

    Vector2 halfGridSize = { gridSize.x / 2 * cubeSize.x, gridSize.y / 2 * cubeSize.z };
    float groundHeight = height - cubeSize.y/2;
    float layerHeight = groundHeight;
    Vector3 position;
    int index, tile;
    for (Json::Value& layer: json["layers"])
    {
        for (int y = 0; y < gridSize.y; y++)
        {
            for (int x = 0; x < gridSize.x; x++)
            {
                int index = y * gridSize.x + x;
                int tile = layer["data"][index].asInt();
                if (tile == 0) // empty tile
                    continue;

                position = {
                    cubeSize.x * x - halfGridSize.x,
                    layerHeight,
                    cubeSize.z * y - halfGridSize.y
                };

                grid[index] = new Cube(position, cubeSize, defaultCubeColor);

                if (tile == 34) // obstruction tile
                {
                    grid[index]->position.y = groundHeight; // put it on the ground so it looks "normal"
                    addObstacle(*grid[index]);
                }
            }
        }

        layerHeight += cubeSize.y; // increment height with one cubeSize.y per layer
    }
}

void Layer::addCube(Vector3 position, Vector3 size, Color color)
{
    grid.push_back(new Cube(position, size, color));
}

Vector3 Layer::getCubeSize()
{
    return cubeSize;
}

Vector2i Layer::getGridSize()
{
    return gridSize;
}

float Layer::getHeight()
{
    return height;
}

std::vector<std::vector<bool>> Layer::getActualObstacles()
{
    return actualObstacles;
}

std::vector<std::vector<bool>> Layer::getTrollObstacles()
{
    return trollObstacles;
}

void Layer::recalculateObstacles()
{
    // this is needed so the player doesn't walk between the edges of two buildings
    // check whether two edges are touching and if they are, fill in the gaps
    std::copy(obstacles.begin(), obstacles.end(), actualObstacles.begin());
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

            if (actualObstacles[topLeft.y][topLeft.x]               // (0, 0 == true)
                && !actualObstacles[topRight.y][topRight.x]         // (0, 1 == false)
                && !actualObstacles[bottomLeft.y][bottomLeft.x]     // (1, 0 == false)
                && actualObstacles[bottomRight.y][bottomRight.x])   // (1, 1 == true)
            {
                actualObstacles[topRight.y][topRight.x] = true;
                actualObstacles[bottomLeft.y][bottomLeft.x] = true;
            }

            else if (!actualObstacles[topLeft.y][topLeft.x]         // (0, 0 == false)
                && actualObstacles[topRight.y][topRight.x]          // (0, 1 == true)
                && actualObstacles[bottomLeft.y][bottomLeft.x]      // (1, 0 == true)
                && !actualObstacles[bottomRight.y][bottomRight.x])  // (1, 1 == false)
            {
                actualObstacles[topLeft.y][topLeft.x] = true;
                actualObstacles[bottomRight.y][bottomRight.x] = true;
            }
        }
    }
}

void Layer::recalculateTrollObstacles()
{
    // shrink original obstacles map by 2 so it can still be used with pathfinding for troll
    // needed to simulate that the troll is twice as big, and shouldn't be able to walk through 1x1 paths
    for (int y = 0; y < gridSize.y - 1; y += 2)
        for (int x = 0; x < gridSize.x - 1; x += 2)
            trollObstacles[y/2][x/2] = (obstacles[y][x] || obstacles[y+1][x] || obstacles[y][x+1] || obstacles[y+1][x+1]);
}

void Layer::addObstacle(Cube cube)
{
    std::vector<Vector2i> indices = getCubeIndices(cube);
    for (Vector2i index: indices)
        obstacles[index.y][index.x] = true;

    recalculateObstacles();
    recalculateTrollObstacles();
}

void Layer::removeObstacle(Cube cube)
{
    std::vector<Vector2i> indices = getCubeIndices(cube);
    for (Vector2i index: indices)
        obstacles[index.y][index.x] = false;

    recalculateObstacles();
    recalculateTrollObstacles();
}

Vector2i Layer::worldPositionToIndex(Vector3 position)
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

Vector3 Layer::indexToWorldPosition(Vector2i index)
{
    Cube* cube = grid[index.y * gridSize.x + index.x];

    Vector2i adjusted = {
        index.x - (gridSize.x/2),
        index.y - (gridSize.y/2),
    };

    return {
        adjusted.x * cubeSize.x,
        cube->position.y + cubeSize.y,
        adjusted.y * cubeSize.z,
    };
}

std::vector<Vector2i> Layer::getCubeIndices(Cube cube)
{
    // this should NOT be simplified to (halfX = maxX / 2) since (3 / 2 = 1) and (3 - (3/2) = 2)
    int maxX = cube.size.x / cubeSize.x;
    int halfX = maxX - maxX / 2;
    int maxY = cube.size.z / cubeSize.z;
    int halfY = maxY - maxY / 2;

    std::vector<Vector2i> indices;
    if (maxX <= 1 && halfX <= 1 && maxY <= 1 && halfY <= 1)
        return std::vector<Vector2i>(1, worldPositionToIndex(cube.position));

    float posX, posZ;
    for (int y = -halfY; y < halfY; y++)
    {
        for (int x = -halfX; x < halfX; x++)
        {
            posX = cube.position.x + (cubeSize.x * x);
            posZ = cube.position.z + (cubeSize.z * y);
            indices.push_back(worldPositionToIndex({ posX, 0.f, posZ }));
        }
    }

    return indices;
}

std::vector<Vector2i> Layer::getNeighboringIndices(std::vector<Vector2i> indices)
{
    std::vector<Vector2i> directions = { { -1, 0}, { 1, 0}, { 0, -1}, { 0, 1}, { -1, -1}, { 1, 1}, { -1, 1}, { 1, -1} };

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

std::vector<Vector2i> Layer::getNeighboringIndices(Cube cube)
{
    std::vector<Vector2i> indices = getCubeIndices(cube);
    return getNeighboringIndices(indices);
}

void Layer::colorTiles(std::list<Vector2i> indices)
{
    for (Cube* cube: grid)
        cube->color = defaultCubeColor;
    for (Vector2i index: indices)
        grid[index.y * gridSize.x + index.x]->color = RED;
}

std::vector<Vector3> Layer::pathfindPositions(Vector3 start, Vector3 goal)
{
    Vector2i startIndex = worldPositionToIndex(start);
    Vector2i goalIndex = worldPositionToIndex(goal);

    std::list<Vector2i> paths = PathFinding::get().findPath(startIndex, goalIndex, getActualObstacles());
    std::vector<Vector3> positions;

    colorTiles(paths);

    for (Vector2i index: paths)
        positions.push_back(indexToWorldPosition(index));

    return positions;
}

std::vector<Vector3> Layer::pathfindPositionsForTroll(Vector3 start, Vector3 goal)
{
    Vector2i startIndex = worldPositionToIndex(start);
    Vector2i goalIndex = worldPositionToIndex(goal);

    Vector2i startTrollIndex = { startIndex.x/2, startIndex.y/2 }; // half to account for troll obstacle map
    Vector2i goalTrollIndex = { goalIndex.x/2, goalIndex.y/2 }; // half to account for troll obstacle map
    std::list<Vector2i> paths = PathFinding::get().findPath(startTrollIndex, goalTrollIndex, getTrollObstacles());
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

Cube* Layer::raycastToGround()
{
    Ray ray = CameraManager::get().getMouseRay();
    float closestCollisionDistance = std::numeric_limits<float>::infinity();
    Cube* nearestCube = nullptr;

    for (Cube* cube: grid)
    {
        RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(*cube));

        if (collision.hit && collision.distance < closestCollisionDistance)
        {
            closestCollisionDistance = collision.distance;
            nearestCube = cube;
        }
    }

    return nearestCube;
}
