#ifndef ENTITY_H
#define ENTITY_H

#include <deque>

#include "utils.h"
#include "structs.h"
#include "Building.h"

enum State { IDLE, RUNNING };
enum EntityType { PLAYER, WORKER };

struct Entity
{
    Capsule capsule;
    Cylinder targetMarker;

    EntityType entityType;
    Color defaultColor;
    Color selectedColor;

    State state;
    State previousState;

    Vector3 speed;
    bool reachedDestination;
    float defaultTargetMargin;
    std::deque<Vector3> path;

    bool selected;

    Entity() = delete;
    Entity(Vector3 position, Color defaultColor, EntityType entityType);
    ~Entity();

    void draw();
    void update();
    void updateMovement();

    Vector3 getPosition();
    void setPath(std::vector<Vector3> newPath);
    void correctPath(std::vector<Vector3> path);
    void setDefaultColor(Color color);
    void setPosition(Vector3 position);
    void setSpeed(Vector3 speed);

    void setState(State newState);

    void select();
    virtual void deselect();
};

#endif
