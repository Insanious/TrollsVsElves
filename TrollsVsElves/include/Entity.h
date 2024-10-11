#ifndef ENTITY_H
#define ENTITY_H

#include <deque>

#include "utils.h"
#include "structs.h"
#include "Building.h"

enum State { IDLE, RUNNING };
enum EntityType { PLAYER, WORKER };

class Entity
{
protected:
    EntityType entityType;
    Color defaultColor;
    Color selectedColor;

    State state;
    State previousState;

    Vector3 speed;
    bool reachedDestination;

    float defaultTargetMargin;

    bool selected;

    Cylinder targetMarker;
    std::deque<Vector3> paths;

public:
    Capsule capsule;

    Entity() = delete;
    Entity(Vector3 position, Color defaultColor, EntityType entityType);
    ~Entity();

    void draw();
    void update();
    void updateMovement();

    Vector3 getPosition();
    void setPositions(std::vector<Vector3> positions);
    void correctPath(std::vector<Vector3> path);
    void setDefaultColor(Color color);
    void setPosition(Vector3 position);
    void setSpeed(Vector3 speed);
    bool hasReachedDestination();

    void setState(State newState);
    State getState();
    State getPreviousState();

    void select();
    virtual void deselect();
    bool isSelected();

    EntityType getEntityType();
};

#endif
