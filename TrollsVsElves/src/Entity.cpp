#include "Entity.h"

Entity::Entity(Capsule capsule, Vector3 speed, EntityType type)
{
    state = IDLE;
    previousState = IDLE;
    selected = false;
    attachedBuilding = nullptr;

    defaultColor = capsule.color;
    Vector3 defaultColorHSL = ColorToHSV(defaultColor);
    selectedColor = ColorFromHSV(defaultColorHSL.x, defaultColorHSL.y, defaultColorHSL.z - 0.2f);

    targetMarker = Cylinder(Vector3Zero(), 4.f, 0.1f, 8, { 255, 255, 255, 30 });

    this->capsule = capsule;
    this->speed = speed;
    this->type = type;
}

Entity::~Entity() {}

void Entity::draw()
{
    drawCapsule(capsule);

    if (state == RUNNING)
        drawCylinder(targetMarker);
}

void Entity::update()
{
    switch (state)
    {
        case IDLE:
            if (attachedBuilding)
                setState(WORKING);
            break;

        case RUNNING:
        case RUNNING_TO_BUILD:
            updateMovement();
            break;

        case WORKING:
            break;
    }
}

void Entity::updateMovement()
{
    if (paths.empty()) // was already standing in the correct location
    {
        setState(IDLE);
        return;
    }

    Vector3 pos = paths.front();
    Vector3 target = { pos.x, 0.f, pos.z };                         // ignore y for now
    Vector3 current = { capsule.endPos.x, 0.f, capsule.endPos.z };  // ignore y for now
    Vector3 direction = Vector3Subtract(target, current);

    Vector3 directionNormalized = Vector3Normalize(direction);
    directionNormalized.y = 0.f;                                    // ignore y for now
    Vector3 velocity = Vector3Scale(Vector3Multiply(directionNormalized, speed), GetFrameTime());

    capsule.startPos = Vector3Add(capsule.startPos, velocity);      // walk to target
    capsule.endPos = Vector3Add(capsule.endPos, velocity);          // walk to target

    float nextDistance = Vector3Distance(target, { capsule.endPos.x, 0.f, capsule.endPos.z });
    if (nextDistance <= defaultTargetMargin)        // will go past target next frame, so just tp to it
    {
        capsule.startPos = { target.x, capsule.startPos.y, target.z };
        capsule.endPos = { target.x, capsule.endPos.y, target.z };

        paths.pop_front(); // reached the end of this path
        if (paths.empty())
            setState(IDLE);
    }
}

Vector3 Entity::getPosition()
{
    return capsule.endPos;
}

Capsule Entity::getCapsule()
{
    return capsule;
}

void Entity::setPositions(std::vector<Vector3> positions, MovementState newState)
{
    paths.clear();
    paths.insert(paths.end(), positions.begin(), positions.end());

    if (paths.size())
        targetMarker.position = { paths.back().x, 2.f, paths.back().z };

    setState(newState);
}

void Entity::setState(MovementState newState)
{
    previousState = state;
    state = newState;

    if (false) // set to true to enable state change logging
    {
        std::string stateString;
        std::string previousStateString;
        switch (state)
        {
            case IDLE:              stateString = "IDLE";               break;
            case RUNNING:           stateString = "RUNNING";            break;
            case RUNNING_TO_BUILD:  stateString = "RUNNING_TO_BUILD";   break;
            case WORKING:           stateString = "WORKING";            break;
        }
        switch (previousState)
        {
            case IDLE:              previousStateString = "IDLE";               break;
            case RUNNING:           previousStateString = "RUNNING";            break;
            case RUNNING_TO_BUILD:  previousStateString = "RUNNING_TO_BUILD";   break;
            case WORKING:           previousStateString = "WORKING";            break;
        }
        printf("newState, previousState: %s, %s\n", stateString.c_str(), previousStateString.c_str());
    }
}

MovementState Entity::getState()
{
    return state;
}

MovementState Entity::getPreviousState()
{
    return previousState;
}

void Entity::select()
{
    selected = true;
    capsule.color = selectedColor;
}

void Entity::deselect()
{
    selected = false;
    capsule.color = defaultColor;
}

bool Entity::isSelected()
{
    return selected;
}

void Entity::attach(Building* building)
{
    attachedBuilding = building;
}

void Entity::detach()
{
    attachedBuilding = nullptr;
}