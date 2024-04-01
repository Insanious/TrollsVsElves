#ifndef LAYER_H
#define LAYER_H

#include <vector>
#include <list>
#include "structs.h"

class Layer
{
private:
    std::vector<Cube*> grid;
    std::vector<std::vector<bool>> obstacles;
    std::vector<std::vector<bool>> actualObstacles;
    Vector2i gridSize;
    Vector3 cubeSize;
    Color defaultCubeColor;
    float height;

public:
    Layer();
    ~Layer();

    void draw();

    void createGrid(Vector2i gridSize, Vector3 cubeSize, Color defaultCubeColor, float height);
    void addCube(Vector3 position, Vector3 size, Color color);
    float getHeight();

    std::vector<std::vector<bool>> getActualObstacles();
    void recalculateObstacles(std::vector<Vector2i> indices);
    void addObstacle(Cube cube);
    void removeObstacle(Cube cube);

    Vector2i worldPositionToIndex(Vector3 position);
    Vector3 indexToWorldPosition(Vector2i index);
    std::vector<Vector2i> getCubeIndices(Cube cube);
    std::vector<Vector2i> getNeighboringIndices(Cube cube);

    void colorTiles(std::list<Vector2i> indices);
};

#endif
