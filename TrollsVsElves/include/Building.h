#ifndef BUILDING_H
#define BUILDING_H

#include "globals.h"

class Building
{
private:
    Cube* cube;

    Color defaultColor;

    bool sold;
    int level;

public:
    Building();
    Building(Vector3 position, Vector3 size, Color color);
    ~Building();

    void draw();

    void setPosition(Vector3 position);
    Vector3 getPosition();
    Cube* getCube();

    void select();
    void deselect();

    void sell();
    bool isSold();
    void upgrade();

    int getLevel();
};

#endif
