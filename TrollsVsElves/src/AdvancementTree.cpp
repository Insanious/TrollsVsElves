#include "AdvancementTree.h"
#include <cassert>

AdvancementTree::AdvancementTree(std::string filename)
{
    Json::Value json = parseJsonFile(filename);

    root = new AdvancementNode(
        IdNode(json["base"].asString(), json["stage"].asInt()),
        json["name"].asString(),
        nullptr,
        {}
    );
    for (const auto& child : json["children"])
        parseNode(child, root);

    if (false) // set to true to log tree structure
        printNode(root, "  -  ");
}

AdvancementTree::~AdvancementTree()
{
    delete root;
}

void AdvancementTree::parseNode(const Json::Value& json, AdvancementNode* parent)
{
    std::vector<IdNode> dependencies;
    for (const auto& dep : json["dependencies"])
        dependencies.push_back(IdNode(dep["base"].asString(), dep["stage"].asInt()));

    AdvancementNode* node = new AdvancementNode(
        IdNode(json["base"].asString(), json["stage"].asInt()),
        json["name"].asString(),
        parent,
        dependencies
    );
    parent->children.push_back(node);

    for (const auto& child : json["children"])
        parseNode(child, node);
}

void AdvancementTree::printNode(AdvancementNode* parent, std::string prefix)
{

    if (parent->dependencies.size())
    {
        printf("%sDependencies:\n", prefix.c_str());
        for (auto dependency: parent->dependencies)
            printIdNode(dependency, prefix + "   ");
    }
    printIdNode(parent->id, prefix);
    printf("%sName: %s\n", prefix.c_str(), parent->name.c_str());

    for (auto child: parent->children)
        printNode(child, prefix + " - ");
}

void AdvancementTree::printIdNode(IdNode id, std::string prefix)
{
    printf("%sBase: %s\n", prefix.c_str(), id.base.c_str());
    printf("%sStage: %d\n", prefix.c_str(), id.stage);
}

AdvancementNode* AdvancementTree::getRoot()
{
    return root;
}
