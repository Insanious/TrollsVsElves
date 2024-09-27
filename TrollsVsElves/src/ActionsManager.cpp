#include "ActionsManager.h"
#include <iostream>

ActionsManager::ActionsManager() {}

ActionsManager::~ActionsManager() {}

void ActionsManager::loadRequirements(std::string filename)
{
    printf("ActionsManager::loadRequirements(%s)\n", filename.c_str());

    Json::Value json = parseJsonFile(filename);
    for (const auto& id: json.getMemberNames())
    {
        std::vector<std::string> reqs;
        for (const auto& req : json[id])
            reqs.push_back(req.asString());

        requirements.insert({ id, reqs });
    }
}

void ActionsManager::loadActions(std::string filename)
{
    printf("ActionsManager::loadActions(%s)\n", filename.c_str());

    Json::Value json = parseJsonFile(filename);
    for (const auto& id: json.getMemberNames())
    {
        printf("loaded %s\n", id.c_str());
        Json::Value& obj = json[id];
        std::string name = obj["name"].asString();
        std::string action = obj["action"].asString();

        std::vector<std::string> children;
        for (const auto& child : obj["children"])
            children.push_back(child.asString());

        actions.insert({ id, ActionNode(id, name, action, children) });
    }
}


std::vector<ActionNode> ActionsManager::getActionChildren(std::string id)
{
    std::vector<ActionNode> childActions;
    for (const auto& id: actions[id].children)
        childActions.push_back(actions[id]);

    return childActions;
}
