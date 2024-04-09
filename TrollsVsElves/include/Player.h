#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"

class Player : public Entity
{
private:
public:
    Player() = delete;
    Player(Capsule capsule, Vector3 speed);
    ~Player();

    void draw();
    void update();
};

#endif
