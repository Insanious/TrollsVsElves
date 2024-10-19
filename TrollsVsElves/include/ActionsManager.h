#ifndef ACTIONS_MANAGER_H
#define ACTIONS_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "utils.h"

struct ActionNode
{
    std::string id;
    std::string name;
    std::string action;
    std::vector<std::string> children;

    bool promotable = false;
    std::function<void()> callback;

    ActionNode():
        id(""),
        name(""),
        action(""),
        children({}) {};

    ActionNode(
        std::string _id,
        std::string _name,
        std::string _action,
        std::vector<std::string> _children
    ):
        id(_id),
        name(_name),
        action(_action),
        children(_children) {};
};

class ActionsManager
{
private:
    ActionsManager();

    std::unordered_map<std::string, ActionNode> actions;
public:
    std::unordered_map<std::string, std::vector<std::string>> requirements;

    ActionsManager(ActionsManager const&) = delete;
    void operator=(ActionsManager const&) = delete;
    ~ActionsManager();

    static ActionsManager& get()
    {
        static ActionsManager instance;
        return instance;
    }

    void loadBuildingFile(std::string filename);
    void loadActions(std::string filename);
    void loadRequirements(std::string filename);

    std::vector<ActionNode> getActionChildren(std::string id);
};

#endif
