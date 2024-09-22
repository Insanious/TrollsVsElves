#ifndef ENTITY_H
#define ENTITY_H

#include <deque>

#include "utils.h"
#include "structs.h"
#include "Building.h"
#include "Resource.h"

enum State {
    IDLE,
    RUNNING,
    RUNNING_TO_BUILD,
    ATTACHED_TO_BUILDING,
    ATTACHED_TO_RESOURCE
};
enum EntityType { PLAYER, WORKER };

class Entity
{
protected:
    EntityType entityType;
    Capsule capsule;
    Color defaultColor;
    Color selectedColor;

    State state;
    State previousState;

    Vector3 speed;

    float defaultTargetMargin;

    bool selected;

    Cylinder targetMarker;
    std::deque<Vector3> paths;

    Building* attachedBuilding;
    Resource* attachedResource;

public:
    Entity() = delete;
    Entity(Vector3 position, Vector3 speed, Color defaultColor, EntityType entityType);
    ~Entity();

    void draw();
    void update();
    void updateMovement();

    Vector3 getPosition();
    Capsule getCapsule();
    void setPositions(std::vector<Vector3> positions, State newState);
    void setDefaultColor(Color color);
    void setCapsule(Capsule capsule);
    void setPosition(Vector3 position);
    void setSpeed(Vector3 speed);
    Vector3 getSpeed();

    void setState(State newState);
    State getState();
    State getPreviousState();

    void select();
    virtual void deselect();
    bool isSelected();

    void attach(Building* building);
    void attach(Resource* resource);
    void detach();

    EntityType getEntityType();
};

#endif
