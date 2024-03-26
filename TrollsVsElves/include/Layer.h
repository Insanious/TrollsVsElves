#ifndef LAYER_H
#define LAYER_H

#include <vector>
#include "structs.h"

class Layer
{
private:
    std::vector<Cube*> grid;
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


};

#endif
