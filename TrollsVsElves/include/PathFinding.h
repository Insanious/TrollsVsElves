#pragma once

#include "structs.h"
#include "MapGenerator.h"
#include "limits.h"
#include <list>
#include <unordered_map>
#include <queue>

namespace AStar
{
    struct Node
    {
        float g = 0;
        float h = 0;
        float f = 0;
        Vector2i pos = { 0 };
        Vector2i parentPos = { 0 };

        Node(): g(), h(), f(), pos(), parentPos() {};
        Node(Vector2i _pos): pos(_pos) {};

        bool operator==(const Node& rhs) const { return this->pos.x == rhs.pos.x && this->pos.y == rhs.pos.y; }
        bool operator!=(const Node& rhs) const { return this->pos.x != rhs.pos.x || this->pos.y != rhs.pos.y; }
        bool operator<(const Node& rhs) const { return this->pos.x < rhs.pos.x && this->pos.y < rhs.pos.y; }

        void print(std::string prefix)
        {
            printf("%s: x: %d, y: %d\tg: %f, h: %f, f: %f\n", prefix.c_str(), pos.x, pos.y, g, h, f);
        }
    };

    struct HashVector2i
    {
        std::size_t operator()(const Vector2i& pos) const { return std::hash<int>()(pos.x) ^ std::hash<int>()(pos.y); }
    };

    struct CompareNode
    {
        bool operator() (const Node& lhs, const Node& rhs) const { return lhs.f > rhs.f; }
    };

    double weightedConvexUpwardParabola(double g, double h);
    double weightedConvexDownwardParabola(double g, double h);
    std::list<Vector2i> backtrack(std::unordered_map<Vector2i, Node, HashVector2i>& nodes, Node& start, Node node);
    std::list<Vector2i> findPath(Vector2i start, Vector2i goal, const std::vector<std::vector<bool>>& obstacles);
}
