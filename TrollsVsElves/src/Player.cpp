#include "Player.h"

Player::Player()
{
    state = IDLE;
    speed = Vector3Zero();
    targetPosition = Vector3Zero();
    defaultTargetMargin = 0.5f;
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
    drawCapsule(&capsule);
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
    if (Vector3Length(direction) > defaultTargetMargin)
    {
        Vector3 directionNormalized = Vector3Normalize(direction);
        Vector3 velocity = Vector3Scale({ directionNormalized.x * speed.x, 0.f, directionNormalized.z * speed.z }, GetFrameTime());

        Vector3 newEndPos = Vector3Add(capsule.endPos, velocity);
        Vector3 nextDirection = Vector3Subtract(target, { newEndPos.x, 0.f, newEndPos.z });
        if (Vector3Length(nextDirection) < defaultTargetMargin) // gonna clip in target, tp to target
        {
            Vector3 targetVector = Vector3Scale(directionNormalized, defaultTargetMargin);
            capsule.startPos = { target.x, capsule.startPos.y, target.z };
            capsule.endPos = { target.x, capsule.endPos.y, target.z };
            state = IDLE;
        }
        else
        {
            capsule.startPos = Vector3Add(capsule.startPos, velocity);
            capsule.endPos = Vector3Add(capsule.endPos, velocity);
        }
    }
    else // reached destination
    {
        state = IDLE;
    }

}

void Player::setTargetPosition(Vector3 position)
{
    state = RUNNING;
    targetPosition = position;
}

Vector3 Player::getPosition()
{
    return capsule.endPos;
}

Capsule Player::getCapsule()
{
    return capsule;
}

bool Player::isIdle()
{
    return state == IDLE;
}

bool Player::isRunning()
{
    return state == RUNNING;
}