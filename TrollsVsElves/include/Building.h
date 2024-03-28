#ifndef BUILDING_H
#define BUILDING_H

#include "structs.h"
#include <cassert>

enum BUILD_STAGE { FLOATING = 0, GHOST, IN_PROGRESS, FINISHED };

class Building
{
private:
    Cube cube;
    BUILD_STAGE buildStage;

    Color floatingColor;
    Color ghostColor;
    Color selectedColor;
    Color targetColor;

    float buildTime;
    float buildTimer;

    bool sold;
    int level;

public:
    Building();
    ~Building();

    void init(Cube cube);
    void draw();
    void update();

    void scheduleBuild();
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
    BUILD_STAGE getBuildStage();
};

#endif
