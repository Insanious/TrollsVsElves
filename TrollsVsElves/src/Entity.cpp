#include "Entity.h"

Entity::Entity(Vector3 position, Vector3 speed, Color defaultColor, EntityType type)
{
    state = IDLE;
    previousState = IDLE;
    selected = false;
    attachedBuilding = nullptr;

    targetMarker = Cylinder(Vector3Zero(), 4.f, 0.1f, 8, { 255, 255, 255, 30 });

    this->speed = speed;
    this->type = type;

    this->defaultTargetMargin = Vector3Length(Vector3Scale(speed, 1/60.f)); // speed divided FPS

    setPosition(position);
    setDefaultColor(defaultColor);
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
            if (paths.empty())
                setState(IDLE);
            else
                updateMovement();

            break;

        case WORKING:
            break;
    }
}

void Entity::updateMovement()
{
    Vector3 target = paths.front();
    Vector3 direction = Vector3Subtract(target, capsule.startPos);
    Vector3 directionNormalized = Vector3Normalize(direction);

    Vector3 velocity = Vector3Scale(Vector3Multiply(directionNormalized, speed), GetFrameTime());

    capsule.startPos = Vector3Add(capsule.startPos, velocity);
    capsule.endPos = Vector3Add(capsule.endPos, velocity);

    float nextDistance = Vector3Distance(target, capsule.startPos);
    if (nextDistance <= defaultTargetMargin) // check if it will go within the target margin
    {
        capsule.startPos = { target.x, capsule.startPos.y, target.z };  // just tp to it
        capsule.endPos = { target.x, capsule.endPos.y, target.z };      // just tp to it

        paths.pop_front(); // reached the end of this path
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

void Entity::setDefaultColor(Color color)
{
    defaultColor = color;
    Vector3 defaultColorHSL = ColorToHSV(defaultColor);
    selectedColor = ColorFromHSV(defaultColorHSL.x, defaultColorHSL.y, defaultColorHSL.z - 0.2f);

    this->capsule.color = defaultColor;
}

void Entity::setCapsule(Capsule capsule)
{
    this->capsule = capsule;
}

void Entity::setPosition(Vector3 position)
{
    capsule.startPos = capsule.endPos = position;
    capsule.endPos.y = capsule.startPos.y + capsule.height;
}

void Entity::setSpeed(Vector3 speed)
{
    this->speed = speed;
    this->defaultTargetMargin = Vector3Length(Vector3Scale(speed, 1/60.f)); // speed divided FPS
}

Vector3 Entity::getSpeed()
{
    return speed;
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

void Entity::attach(Resource* resource)
{
    attachedResource = resource;
}

void Entity::detach()
{
    attachedBuilding = nullptr;
    attachedResource = nullptr;
}

EntityType Entity::getType()
{
    return type;
}