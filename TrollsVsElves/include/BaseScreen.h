#ifndef BASE_SCREEN_H
#define BASE_SCREEN_H

#include "structs.h"
class BaseScreen
{
private:

public:
    BaseScreen();
    BaseScreen(Vector2i screenSize);
    ~BaseScreen();

    virtual void draw() = 0;
    virtual void update() = 0;
};

#endif
