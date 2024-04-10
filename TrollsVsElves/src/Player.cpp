#include "Player.h"

Player::Player(Capsule capsule, Vector3 speed)
    : Entity(capsule, speed, PLAYER) {}

Player::~Player() {}

void Player::draw()
{
    Entity::draw();
}

void Player::update()
{
    Entity::update();
}
