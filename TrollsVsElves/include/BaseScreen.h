#ifndef BASE_SCREEN_H
#define BASE_SCREEN_H

#include "globals.h"
class BaseScreen
{
private:

public:
    BaseScreen();
    ~BaseScreen();

    virtual void draw() = 0;
    virtual void init(Vector2i screenSize) = 0;
    virtual void update() = 0;
};

#endif
