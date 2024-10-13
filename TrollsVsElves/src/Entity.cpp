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
    Vector3 target = path.front();
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

        path.pop_front(); // reached the end of this path
        if (path.empty())
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

void Entity::setPath(std::vector<Vector3> newPath)
{
    path.clear();
    path.insert(path.end(), newPath.begin(), newPath.end());

    // TODO, WARNING
    // When no path was found from the pathfinding this handling is incorrect,
    // however this handling is correct when the player is standing at the desired position...
    if (path.empty()) // was ordered to walk somewhere but was already there, destination has been reached good sir
    {
        setState(IDLE);
        reachedDestination = true;
    }
    else
    {
        targetMarker.position = { path.back().x, 2.f, path.back().z };
        setState(RUNNING);
    }
}

void Entity::correctPath(std::vector<Vector3> path)
{
    // TODO: do some actual correction logic here
    setPath(path);
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
    this->defaultTargetMargin = Vector3Length(Vector3Scale(speed, 1/60.f)); // speed divided by frametime
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
