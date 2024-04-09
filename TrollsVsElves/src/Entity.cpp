#include "Entity.h"

Entity::Entity(Capsule capsule, Vector3 speed)
{
    state = IDLE;
    previousState = IDLE;
    selected = false;

    defaultColor = capsule.color;
    Vector3 defaultColorHSL = ColorToHSV(defaultColor);
    selectedColor = ColorFromHSV(defaultColorHSL.x, defaultColorHSL.y, defaultColorHSL.z - 0.2f);

    targetMarker = Cylinder(Vector3Zero(), 4.f, 0.1f, 8, { 255, 255, 255, 30 });

    this->capsule = capsule;
    this->speed = speed;
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
    if (state != IDLE)
        updateMovement();
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

        paths.pop_front();// reached the end of this path
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
