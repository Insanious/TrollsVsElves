#include "Player.h"

Player::Player()
{
    state = IDLE;
    previousState = IDLE;
    speed = Vector3Zero();

    defaultTargetMargin = 0.5f;
    targetPosition = Vector3Zero();
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
    updateMovement();
}

void Player::updateMovement()
{
    Vector3 target = { targetPosition.x, 0.f, targetPosition.z }; // ignore y for now
    Vector3 current = { capsule.endPos.x, 0.f, capsule.endPos.z }; // ignore y for now
    Vector3 direction = Vector3Subtract(target, current);

    if (Vector3Length(direction) <= defaultTargetMargin) return; // target already reached, do nothing

    Vector3 directionNormalized = Vector3Normalize(direction);
    directionNormalized.y = 0.f; // ignore y for now
    Vector3 velocity = Vector3Scale(Vector3Multiply(directionNormalized, speed), GetFrameTime());

    Vector3 newEndPos = Vector3Add(capsule.endPos, velocity);
    Vector3 nextDirection = Vector3Subtract(target, { newEndPos.x, 0.f, newEndPos.z });
    if (Vector3Length(nextDirection) > defaultTargetMargin) // keep walking to target
    {
        capsule.startPos = Vector3Add(capsule.startPos, velocity);
        capsule.endPos = Vector3Add(capsule.endPos, velocity);
        return;
    }

    // gonna clip in target, just tp to it
    Vector3 targetVector = Vector3Scale(directionNormalized, defaultTargetMargin);
    capsule.startPos = { target.x, capsule.startPos.y, target.z };
    capsule.endPos = { target.x, capsule.endPos.y, target.z };
    setState(IDLE);
}

void Player::setTargetPosition(Vector3 position)
{
    if (state != RUNNING)
        setState(RUNNING);

    targetPosition = position;
    targetMarker.position = { position.x, 2.f, position.z };
}

Vector3 Player::getPosition()
{
    return capsule.endPos;
}

Capsule Player::getCapsule()
{
    return capsule;
}

void Player::setState(PLAYER_STATE newState)
{
    previousState = state;
    state = newState;
}

PLAYER_STATE Player::getState()
{
    return state;
}

PLAYER_STATE Player::getPreviousState()
{
    return previousState;
}
