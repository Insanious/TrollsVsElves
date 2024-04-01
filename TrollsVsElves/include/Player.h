#ifndef PLAYER_H
#define PLAYER_H

#include <cmath>
#include <deque>

#include "utils.h"
#include "structs.h"

#include "Building.h"

enum PLAYER_STATE { IDLE, RUNNING, RUNNING_TO_BUILD };

class Player
{
private:
    Capsule capsule;
    PLAYER_STATE state;
    PLAYER_STATE previousState;

    Vector3 speed;

    float defaultTargetMargin;
    Cylinder targetMarker;
    std::deque<Vector3> paths;

public:
    Player();
    ~Player();

    void init(Capsule capsule, Vector3 speed);
    void draw();
    void update();
    void updateMovement();

    Vector3 getPosition();
    Capsule getCapsule();
    void setPositions(std::vector<Vector3> positions, PLAYER_STATE newState);

    void setState(PLAYER_STATE newState);
    PLAYER_STATE getState();
    PLAYER_STATE getPreviousState();
};

#endif
