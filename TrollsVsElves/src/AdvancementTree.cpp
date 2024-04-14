#include "AdvancementTree.h"
#include <cassert>

AdvancementTree::AdvancementTree(std::string filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        printf("couldn't open file %s\n", filename.c_str());
        assert(file.is_open());
    }

    Json::Value json;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(file, json);
    file.close();

    if (!parsingSuccessful)
    {
        printf("couldn't parse json file %s\n", filename.c_str());
        assert(parsingSuccessful);
    }

    root = new AdvancementNode(nullptr, json["id"].asString(), json["name"].asString(), {});
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
    std::vector<std::string> dependencies;
    for (const auto& dependency : json["dependencies"])
        dependencies.push_back(dependency.asString());

    AdvancementNode* node = new AdvancementNode(
        parent,
        json["id"].asString(),
        json["name"].asString(),
        dependencies
    );
    parent->children.push_back(node);

    for (const auto& child : json["children"])
        parseNode(child, node);
}

void AdvancementTree::printNode(AdvancementNode* parent, std::string prefix)
{
    printf("%sId: %s\n", prefix.c_str(), parent->id.c_str());
    printf("%sName: %s\n", prefix.c_str(), parent->name.c_str());

    if (parent->dependencies.size())
    {
        printf("%sDependencies:\n", prefix.c_str());
        for (auto dependency: parent->dependencies)
            printf("%s%s\n", prefix.c_str(), dependency.c_str());
    }


    for (auto child: parent->children)
        printNode(child, prefix + " - ");
}

AdvancementNode* AdvancementTree::getRoot()
{
    return root;
}
