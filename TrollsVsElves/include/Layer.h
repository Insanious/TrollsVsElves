#ifndef LAYER_H
#define LAYER_H

#include <vector>
#include <list>

#include "structs.h"
#include "PathFinding.h"
#include "CameraManager.h"

class Layer
{
private:
    std::vector<Cube*> grid;
    std::vector<std::vector<bool>> obstacles;
    std::vector<std::vector<bool>> actualObstacles;
    std::vector<std::vector<bool>> trollObstacles;
    Vector2i gridSize;
    Vector3 cubeSize;
    Color defaultCubeColor;
    float height;

public:
    Layer();
    ~Layer();

    void draw();

    void createFromFile(std::string filename);
    void addCube(Vector3 position, Vector3 size, Color color);
    Vector3 getCubeSize();
    Vector2i getGridSize();
    float getHeight();

    Cube* raycastToGround();

    std::vector<std::vector<bool>> getActualObstacles();
    std::vector<std::vector<bool>> getTrollObstacles();
    void recalculateObstacles();
    void recalculateTrollObstacles();
    void addObstacle(Cube cube);
    void removeObstacle(Cube cube);

    Vector2i worldPositionToIndex(Vector3 position);
    Vector3 indexToWorldPosition(Vector2i index);
    std::vector<Vector2i> getCubeIndices(Cube cube);
    std::vector<Vector2i> getNeighboringIndices(std::vector<Vector2i> indices);
    std::vector<Vector2i> getNeighboringIndices(Cube cube);

    void colorTiles(std::list<Vector2i> indices);
    std::vector<Vector3> pathfindPositions(Vector3 start, Vector3 goal);
    std::vector<Vector3> pathfindPositionsForTroll(Vector3 start, Vector3 goal);
};

#endif
