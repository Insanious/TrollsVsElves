#ifndef ENTITY_H
#define ENTITY_H

#include <deque>

#include "utils.h"
#include "structs.h"
#include "Building.h"
#include "Resource.h"

enum MovementState {
    IDLE,
    RUNNING,
    RUNNING_TO_BUILD,
    WORKING
};
enum EntityType {
    PLAYER,
    WORKER
};

class Entity
{
private:
    EntityType type;
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

    Building* attachedBuilding;
    Resource* attachedResource;

public:
    Entity() = delete;
    Entity(Capsule capsule, Vector3 speed, EntityType type);
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
    virtual void deselect();
    bool isSelected();

    void attach(Building* building);
    void attach(Resource* resource);
    void detach();

    EntityType getType();
};

#endif
