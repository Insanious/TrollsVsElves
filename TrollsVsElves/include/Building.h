#ifndef BUILDING_H
#define BUILDING_H

#include "structs.h"
#include "imgui.h"
#include "rlImGui.h"
#include "AdvancementTree.h"
#include "Signal.h"

enum BuildStage { GHOST = 0, SCHEDULED, IN_PROGRESS, FINISHED };
enum BuildingType { CASTLE, ROCK, COUNT };

class Building
{
private:
    Cube cube;
    BuildStage buildStage;
    BuildingType buildingType;

    Color ghostColor;
    Color inProgressColor;
    Color selectedColor;
    Color targetColor;

    float buildTime;
    float buildTimer;

    bool selected;
    bool sold;
    bool canRecruit;
    bool recruiting;

    Cylinder rallyPoint;

    AdvancementNode* advancement;
    std::vector<std::string> lockedPromotions;

    simpleSignal::Signal<void(std::string)>* promotionSignal;

    void checkKeyboardPresses(std::vector<AdvancementNode*> children);
    void handleButtonPressLogic(AdvancementNode* node);

public:
    Building() = delete;
    Building(Cube cube, BuildingType buildingType, AdvancementNode* options, simpleSignal::Signal<void(std::string)>* promotionSignal);
    ~Building();

    void draw();
    void drawUIButtons(ImVec2 buttonSize, int nrOfButtons, int buttonsPerLine);
    void update();

    void scheduleBuild();
    void build();

    void setPosition(Vector3 position);
    Vector3 getPosition();
    Cube& getCube();
    Color getGhostColor();

    void select();
    void deselect();
    bool isSelected();

    void sell();
    bool isSold();
    bool isRecruiting();
    void setRecruiting(bool value);

    Cylinder getRallyPoint();
    void setRallyPoint(Vector3 point);

    std::vector<AdvancementNode*> getPossiblePromotions();
    bool canBePromotedTo(AdvancementNode* promotion);
    void promote(AdvancementNode* promotion);

    void updateLockedPromotions(std::vector<std::string> lockedPromotions);
};

#endif
