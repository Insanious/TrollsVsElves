#ifndef BUILDING_H
#define BUILDING_H

#include "structs.h"
#include <cassert>

enum BUILD_STAGE { GHOST = 0, IN_PROGRESS, FINISHED };

class Building
{
private:
    Cube cube;
    BUILD_STAGE buildStage;

    Color defaultColor;

    bool sold;
    int level;

public:
    Building();
    Building(Vector3 position, Vector3 size, Color color);
    ~Building();

    void draw();

    void build();

    void setPosition(Vector3 position);
    Vector3 getPosition();
    Cube& getCube();

    void select();
    void deselect();

    void sell();
    bool isSold();
    void upgrade();

    int getLevel();
};

#endif
