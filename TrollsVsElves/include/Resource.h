#ifndef RESOURCE_H
#define RESOURCE_H

#include "structs.h"

class Resource
{
private:
    Cube cube;

    Color selectedColor;
    Color defaultColor;

    bool selected;

public:
    Resource() = delete;
    Resource(Cube cube);
    ~Resource();

    void draw();
};

#endif
