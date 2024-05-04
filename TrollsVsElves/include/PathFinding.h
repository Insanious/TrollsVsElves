#ifndef PATH_FINDING_H
#define PATH_FINDING_H

#include "structs.h"
#include "MapGenerator.h"
#include "limits.h"
#include <list>
#include <unordered_map>
#include <queue>

struct Node
{
    float g = 0;
    float h = 0;
    float f = 0;
    Vector2i position = { 0 };
    Vector2i parentPosition = { 0 };

    Node(): g(), h(), f(), position(), parentPosition() {};
    Node(Vector2i _position): position(_position) {};

    bool operator==(const Node& rhs) const
    {
        return this->position.x == rhs.position.x && this->position.y == rhs.position.y;
    }
    bool operator!=(const Node& rhs) const
    {
        return this->position.x != rhs.position.x || this->position.y != rhs.position.y;
    }
    bool operator<(const Node& rhs) const
    {
        return this->position.x < rhs.position.x && this->position.y < rhs.position.y;
    }
};

struct HashVector2i
{
    std::size_t operator()(const Vector2i& pos) const
    {
        return std::hash<int>()(pos.x) ^ std::hash<int>()(pos.y);
    }
};

struct CompareNode
{
    bool operator() (const Node& lhs, const Node& rhs) const
    {
        return lhs.f > rhs.f;
    }
};

class PathFinding
{
private:
    PathFinding() {}

    void printNode(std::string prefix, Node node);
    // refer to https://www.movingai.com/SAS/SUB/ for these calculated values
    double weightedConvexUpwardParabola(double g, double h);
    double weightedConvexDownwardParabola(double g, double h);

public:
    PathFinding(PathFinding const&) = delete;
    void operator=(PathFinding const&) = delete;
    ~PathFinding() {}
    static PathFinding& get()
    {
        static PathFinding instance;
        return instance;
    }

    std::list<Vector2i> findPath(Vector2i start, Vector2i goal, const std::vector<std::vector<bool>> obstacles);
};

#endif
