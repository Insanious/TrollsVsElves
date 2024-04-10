#include "Layer.h"

Layer::Layer()
{
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

void Layer::createGrid(Vector2i gridSize, Vector3 cubeSize, Color defaultCubeColor, float height)
{
    this->gridSize = gridSize;
    this->cubeSize = cubeSize;
    this->defaultCubeColor = defaultCubeColor;
    this->height = height;

    obstacles = std::vector<std::vector<bool>>(gridSize.x, std::vector<bool>(gridSize.y, false));
    actualObstacles = std::vector<std::vector<bool>>(gridSize.x, std::vector<bool>(gridSize.y, false));

    Vector2 halfGridSize = { gridSize.x/2 * cubeSize.x, gridSize.y/2 * cubeSize.y };
    for (int z = 0; z < gridSize.y; z++)
    {
        for (int x = 0; x < gridSize.x; x++)
        {
            Vector3 position = { cubeSize.x * x - halfGridSize.x, height - cubeSize.y/2, cubeSize.z * z - halfGridSize.y, };
            grid.push_back(new Cube(position, cubeSize, defaultCubeColor));
        }
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

float Layer::getHeight()
{
    return height;
}

std::vector<std::vector<bool>> Layer::getActualObstacles()
{
    return actualObstacles;
}

void Layer::recalculateObstacles(std::vector<Vector2i> indices)
{
    // this is needed so the player doesn't walk between the edges of two buildings
    // check whether two edges are touching and if they are, fill in the gaps
    std::copy(obstacles.begin(), obstacles.end(), actualObstacles.begin());
    for (int y = 0; y < gridSize.y - 1; ++y)
    {
        for (int x = 0; x < gridSize.x - 1; ++x)
        {
            Vector2i topLeft        = { x,      y };                // index (0, 0)
            Vector2i topRight       = { x + 1,  y };                // index (0, 1)
            Vector2i bottomLeft     = { x,      y + 1 };            // index (1, 0)
            Vector2i bottomRight    = { x + 1,  y + 1 };            // index (1, 1)

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

void Layer::addObstacle(Cube cube)
{
    std::vector<Vector2i> indices = getCubeIndices(cube);
    for (Vector2i index: indices)
        obstacles[index.y][index.x] = true;
    recalculateObstacles(indices);
}

void Layer::removeObstacle(Cube cube)
{
    std::vector<Vector2i> indices = getCubeIndices(cube);
    for (Vector2i index: indices)
        obstacles[index.y][index.x] = false;
    recalculateObstacles(indices);
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
    Vector2i adjusted = {
        index.x - (gridSize.x/2),
        index.y - (gridSize.y/2),
    };
    return {
        adjusted.x * cubeSize.x,
        height,
        adjusted.y * cubeSize.z,
    };
}

bool Layer::worldPositionWithinBounds(Vector3 position)
{
    Vector2i index = worldPositionToIndex(position);
    return index.x >= 0 && index.x < gridSize.x && index.y >= 0 && index.y < gridSize.y;
}

std::vector<Vector2i> Layer::getCubeIndices(Cube cube)
{
    // this should NOT be simplified to (halfX = maxX / 2) since (3 / 2 = 1) and (3 - (3/2) = 2)
    int maxX = cube.size.x / cubeSize.x;
    int halfX = maxX - maxX / 2;
    int maxY = cube.size.z / cubeSize.z;
    int halfY = maxY - maxY / 2;

    std::vector<Vector2i> indices;
    for (int y = -halfY; y < halfY; y++)
    {
        for (int x = -halfX; x < halfX; x++)
        {
            indices.push_back(worldPositionToIndex({
                (cube.position.x) + (cubeSize.x * x),
                0.f,
                (cube.position.z) + (cubeSize.z * y)
            }));
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