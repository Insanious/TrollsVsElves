#ifndef BASE_SCREEN_H
#define BASE_SCREEN_H

class BaseScreen
{
private:

public:
    BaseScreen();
    ~BaseScreen();

    virtual void draw() = 0;
    virtual void init() = 0;
    virtual void update() = 0;
};

#endif
