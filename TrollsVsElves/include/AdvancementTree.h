#ifndef ADVANCEMENT_TREE_H
#define ADVANCEMENT_TREE_H

#include <string>
#include <vector>
#include "utils.h"

struct IdNode
{
    std::string base;
    int stage;

    IdNode(std::string _base, int _stage): base(_base), stage(_stage) {};
    IdNode(): base(), stage() {};
};

struct AdvancementNode
{
    IdNode id;
    std::string name;
    AdvancementNode* parent;
    std::vector<IdNode> dependencies;
    std::vector<AdvancementNode*> children;

    AdvancementNode(IdNode _id, std::string _name, AdvancementNode* _parent, std::vector<IdNode> _dependencies):
        id(_id), name(_name), parent(_parent), dependencies(_dependencies) {};

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
    void printIdNode(IdNode id, std::string prefix);

public:
    AdvancementTree() = delete;
    AdvancementTree(std::string filename);
    ~AdvancementTree();

    AdvancementNode* getRoot();
};

#endif
