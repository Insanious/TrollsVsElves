#ifndef PLAYER_H
#define PLAYER_H

#include <cmath>

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

    Vector3 targetPosition;
    float defaultTargetMargin;
public:
    Player();
    ~Player();

    void init(Capsule capsule, Vector3 speed);
    void draw();
    void update();
    void updateMovement();

    void setTargetPosition(Vector3 position);
    Vector3 getPosition();
    Capsule getCapsule();

    void setState(PLAYER_STATE newState);
    PLAYER_STATE getState();
    PLAYER_STATE getPreviousState();

    void setRunningToBuild();
};

#endif
