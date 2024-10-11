#include "Entity.h"

Entity::Entity(Vector3 position, Color defaultColor, EntityType entityType)
{
    state = IDLE;
    previousState = IDLE;
    selected = false;
    reachedDestination = false;

    targetMarker = Cylinder(Vector3Zero(), 4.f, 0.1f, 8, { 255, 255, 255, 30 });

    this->entityType = entityType;

    this->defaultTargetMargin = Vector3Length(Vector3Scale(speed, 1/60.f)); // speed divided FPS

    setPosition(position);
    setDefaultColor(defaultColor);
    setSpeed(Vector3Scale(Vector3One(), 40));
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
    if (state == RUNNING) {
        updateMovement();
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
        if (paths.empty())
        {
            setState(IDLE);
            reachedDestination = true;
        }
    }
}

Vector3 Entity::getPosition()
{
    return capsule.startPos;
}

void Entity::setPositions(std::vector<Vector3> positions)
{
    paths.clear();
    paths.insert(paths.end(), positions.begin(), positions.end());

    if (paths.size())
    {
        targetMarker.position = { paths.back().x, 2.f, paths.back().z };
        setState(RUNNING);
    }
}

void Entity::correctPath(std::vector<Vector3> path)
{
    // TODO: do some actual correction logic here
    setPositions(path);
}

void Entity::setDefaultColor(Color color)
{
    defaultColor = color;
    Vector3 defaultColorHSL = ColorToHSV(defaultColor);
    selectedColor = ColorFromHSV(defaultColorHSL.x, defaultColorHSL.y, defaultColorHSL.z - 0.2f);

    this->capsule.color = defaultColor;
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

bool Entity::hasReachedDestination()
{
    bool value = reachedDestination;
    reachedDestination = false; // one-shot read true
    return value;
}

void Entity::setState(State newState)
{
    previousState = state;
    state = newState;

    if (false) // set to true to enable state change logging
    {
        std::string stateString;
        std::string previousStateString;
        switch (state)
        {
            case IDLE:      stateString = "IDLE";       break;
            case RUNNING:   stateString = "RUNNING";    break;
        }
        switch (previousState)
        {
            case IDLE:      previousStateString = "IDLE";       break;
            case RUNNING:   previousStateString = "RUNNING";    break;
        }
        printf("newState, previousState: %s, %s\n", stateString.c_str(), previousStateString.c_str());
    }
}

State Entity::getState()
{
    return state;
}

State Entity::getPreviousState()
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

EntityType Entity::getEntityType()
{
    return entityType;
}