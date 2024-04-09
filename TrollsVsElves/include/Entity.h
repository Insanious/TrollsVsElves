#ifndef ENTITY_H
#define ENTITY_H

#include <deque>

#include "utils.h"
#include "structs.h"

enum MovementState { IDLE, RUNNING, RUNNING_TO_BUILD };

class Entity
{
private:
    Capsule capsule;
    Color defaultColor;
    Color selectedColor;

    MovementState state;
    MovementState previousState;

    Vector3 speed;

    const float defaultTargetMargin = 0.5f;

    bool selected;

    Cylinder targetMarker;
    std::deque<Vector3> paths;

public:
    Entity() = delete;
    Entity(Capsule capsule, Vector3 speed);
    ~Entity();

    void draw();
    void update();
    void updateMovement();

    Vector3 getPosition();
    Capsule getCapsule();
    void setPositions(std::vector<Vector3> positions, MovementState newState);

    void setState(MovementState newState);
    MovementState getState();
    MovementState getPreviousState();

    void select();
    void deselect();
    bool isSelected();
};

#endif
