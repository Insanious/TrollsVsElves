#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H

#include <vector>
#include <list>

#include "structs.h"
#include "PathFinding.h"
#include "CameraManager.h"

struct MapGenerator
{
    std::vector<Cube> grid;
    std::vector<std::vector<bool>> obstacles;
    std::vector<std::vector<bool>> elfObstacles;
    std::vector<std::vector<bool>> trollObstacles;
    Vector2i gridSize;
    Vector3 cubeSize;
    Color defaultCubeColor;
    float height;

    MapGenerator();
    ~MapGenerator();

    void draw();

    void generateFromFile(std::string filename);

    Cube* raycastToGround();

    void recalculateObstacles();
    void recalculateTrollObstacles();
    void addObstacle(Cube cube);
    void removeObstacle(Cube cube);

    int twoDimToOneDimIndex(Vector2i index);
    Vector2i worldPositionToIndex(Vector3 position);
    Vector3 indexToWorldPosition(Vector2i index);
    Vector3 worldPositionAdjusted(Vector3 position);
    std::vector<Vector2i> getCubeIndices(Cube& cube);
    std::vector<Vector2i> getNeighboringIndices(std::vector<Vector2i> indices);
    std::vector<Vector2i> getNeighboringIndices(Cube cube);

    void colorTiles(std::list<Vector2i> indices);
    std::vector<Vector3> pathfindPositionsForElf(Vector3 start, Vector3 goal);
    std::vector<Vector3> pathfindPositionsForTroll(Vector3 start, Vector3 goal);
};

#endif
