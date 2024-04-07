#ifndef PLAYER_H
#define PLAYER_H

#include <cmath>
#include <deque>

#include "utils.h"
#include "structs.h"

#include "Building.h"

enum PlayerState { IDLE, RUNNING, RUNNING_TO_BUILD };

class Player
{
private:
    Capsule capsule;
    PlayerState state;
    PlayerState previousState;

    Vector3 speed;

    bool selected;

    float defaultTargetMargin;
    Cylinder targetMarker;
    std::deque<Vector3> paths;

public:
    Player() = delete;
    Player(Capsule capsule, Vector3 speed);
    ~Player();

    void draw();
    void update();
    void updateMovement();

    Vector3 getPosition();
    Capsule getCapsule();
    void setPositions(std::vector<Vector3> positions, PlayerState newState);

    void setState(PlayerState newState);
    PlayerState getState();
    PlayerState getPreviousState();

    void select();
    void deselect();
    bool isSelected();
};

#endif
