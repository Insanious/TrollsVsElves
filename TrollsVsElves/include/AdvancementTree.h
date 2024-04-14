#ifndef ADVANCEMENT_TREE_H
#define ADVANCEMENT_TREE_H

#include <string>
#include <vector>
#include <json/json.h>
#include <fstream>

struct AdvancementNode
{
    std::string id;
    std::string name;
    AdvancementNode* parent;
    std::vector<std::string> dependencies;
    std::vector<AdvancementNode*> children;

    AdvancementNode(AdvancementNode* _parent, std::string _id, std::string _name, std::vector<std::string> _dependencies):
        parent(_parent), id(_id), name(_name), dependencies(_dependencies) {};

    ~AdvancementNode()
    {
        for (int i = 0; i < children.size(); i++)
            delete children[i];
    };
};

class AdvancementTree
{
private:
    AdvancementNode* root;

    void parseNode(const Json::Value& json, AdvancementNode* parent);
    void printNode(AdvancementNode* parent, std::string prefix);

public:
    AdvancementTree() = delete;
    AdvancementTree(std::string filename);
    ~AdvancementTree();

    AdvancementNode* getRoot();
};

#endif
