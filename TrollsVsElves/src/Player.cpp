#include "Player.h"

Player::Player()
{
    state = IDLE;
    previousState = IDLE;
    speed = Vector3Zero();
    selected = false;

    defaultTargetMargin = 0.5f;
    targetMarker = Cylinder(Vector3Zero(), 4.f, 0.1f, 8, { 255, 255, 255, 30 });
}

Player::~Player()
{
}

void Player::init(Capsule capsule, Vector3 speed)
{
    this->capsule = capsule;
    this->speed = speed;
}

void Player::draw()
{
    drawCapsule(capsule);
    if (state == RUNNING)
        drawCylinder(targetMarker);

}

void Player::update()
{
    if (state != IDLE) updateMovement();
}

void Player::updateMovement()
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

    Vector3 newEndPos = Vector3Add(capsule.endPos, velocity);
    Vector3 nextDirection = Vector3Subtract(target, { newEndPos.x, 0.f, newEndPos.z });

    if (Vector3Length(nextDirection) > defaultTargetMargin)         // keep walking to target
    {
        capsule.startPos = Vector3Add(capsule.startPos, velocity);
        capsule.endPos = Vector3Add(capsule.endPos, velocity);
        return;
    }

    // gonna clip in target, just tp to it
    capsule.startPos = { target.x, capsule.startPos.y, target.z };
    capsule.endPos = { target.x, capsule.endPos.y, target.z };

    paths.pop_front(); // reached the end of this path
    if (paths.empty())
        setState(IDLE);
}

Vector3 Player::getPosition()
{
    return capsule.endPos;
}

Capsule Player::getCapsule()
{
    return capsule;
}

void Player::setPositions(std::vector<Vector3> positions, PlayerState newState)
{
    paths.clear();
    paths.insert(paths.end(), positions.begin(), positions.end());
    if (paths.size())
        targetMarker.position = { paths.back().x, 2.f, paths.back().z };

    setState(newState);
}

void Player::setState(PlayerState newState)
{
    previousState = state;
    state = newState;
}

PlayerState Player::getState()
{
    return state;
}

PlayerState Player::getPreviousState()
{
    return previousState;
}

void Player::select()
{
    selected = true;
}

void Player::deselect()
{
    selected = false;
}

bool Player::isSelected()
{
    return selected;
}
