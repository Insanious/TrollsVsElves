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
        drawCube(cube);
}


// Cube* Layer::raycastToClosestCube(Camera camera, std::vector<Cube*> cubes)
// {
//     Ray ray = GetMouseRay(GetMousePosition(), camera);
//     float closestCollisionDistance = std::numeric_limits<float>::infinity();;
//     Cube* closestCube = nullptr;

//     for (Cube* cube: grid)
//     {
//         RayCollision collision = GetRayCollisionBox(ray, getCubeBoundingBox(*cube));

//         if (collision.hit && collision.distance < closestCollisionDistance) {
//             closestCollisionDistance = collision.distance;
//             closestCube = cube;
//         }
//     }

//     return closestCube;
// }

void Layer::createGrid(Vector2i gridSize, Vector3 cubeSize, Color defaultCubeColor, float height)
{
    this->gridSize = gridSize;
    this->cubeSize = cubeSize;
    this->defaultCubeColor = defaultCubeColor;
    this->height = height;

    Vector2 halfGridSize = { gridSize.x/2 * cubeSize.x, gridSize.y/2 * cubeSize.y };
    for (int x = 0; x < gridSize.x; x++)
    {
        for (int z = 0; z < gridSize.y; z++)
        {
            Vector3 position = { cubeSize.x * x - halfGridSize.x, height, cubeSize.z * z - halfGridSize.y, };
            // Vector3 size = { cubeSize, cubeSize, cubeSize };
            int index = x + z * gridSize.x;
            grid.push_back(new Cube(position, cubeSize, defaultCubeColor));
        }
    }
}

void Layer::addCube(Vector3 position, Vector3 size, Color color)
{
    grid.push_back(new Cube(position, size, color));
}

float Layer::getHeight()
{
    return height;
}