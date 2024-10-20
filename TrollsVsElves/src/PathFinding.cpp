#include "PathFinding.h"

#include "time.h" // time_t
#include <iomanip>
#include <iostream>
#include <limits>

namespace AStar
{
    double weightedConvexUpwardParabola(double g, double h)
    {
        double weight = 2.f;
        double numerator = g + h + sqrt(pow(g + h, 2) + 4 * weight * (weight - 1) * pow(h, 2));
        double denominator = 2 * weight;
        return numerator / denominator;
    }

    double weightedConvexDownwardParabola(double g, double h)
    {
        double weight = 2.f;
        double numerator = g + (2 * weight - 1) * h + sqrt(pow(g - h, 2) + 4 * weight * g * h);
        double denominator = 2 * weight;
        return numerator / denominator;
    }

    std::list<Vector2i> backtrack(std::unordered_map<Vector2i, Node, HashVector2i>& nodes, Node& start, Node node)
    {
        std::list<Vector2i> path;
        while (node != start)
        {
            path.push_front(node.pos);
            node = nodes[node.parentPos];
        }
        return path;
    }

    std::list<Vector2i> findPath(Vector2i start, Vector2i goal, const std::vector<std::vector<bool>>& obstacles)
    {
        clock_t start_t = clock();      // for debugging purposes
        int allocations = 0;            // for debugging purposes
        bool printDebugInfo = false;    // for debugging purposes

        int count = 0;
        int maxY = obstacles.size();
        int maxX = obstacles[0].size();
        Node current, neighbor;
        Node startNode = Node(start);
        Node goalNode = Node(goal);
        Vector2i neighborPositions[8] = { { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 } };
        static const int directions[8][2] = {
            {-1,  0},
            { 1,  0},
            { 0, -1},
            { 0,  1},

            {-1, -1}, // diagonals
            { 1,  1}, // diagonals
            {-1,  1}, // diagonals
            { 1, -1}, // diagonals
        };

        std::priority_queue<Node, std::vector<Node>, CompareNode> notVisitedHeap;
        std::unordered_map<Vector2i, Node, HashVector2i> notVisitedMap;
        std::unordered_map<Vector2i, Node, HashVector2i> visited;

        notVisitedHeap.push(startNode);
        while (!notVisitedHeap.empty())
        {
            current = notVisitedHeap.top();
            notVisitedHeap.pop();

            visited[current.pos] = current;

            if (current == goalNode)
            {
                if (printDebugInfo)
                    printf("time, allocations, visited: %f, %d, %d\n", double(clock()-start_t)/CLOCKS_PER_SEC, allocations, visited.size());

                return backtrack(visited, startNode, current);
            }

            for (int i = 0; i < 8; ++i) // calculate new positions
            {
                neighborPositions[i].x = current.pos.x + directions[i][0];
                neighborPositions[i].y = current.pos.y + directions[i][1];
            }

            count = -1;
            for (Vector2i& pos: neighborPositions)
            {
                count++;
                if (pos.x < 0 || pos.x >= maxX || pos.y < 0 || pos.y >= maxY    // out of bounds
                    || visited.find(pos) != visited.end()                       // already visited
                    || notVisitedMap.find(pos) != notVisitedMap.end()           // already in queue
                    || obstacles[pos.y][pos.x])                                 // is not traversable
                    continue;

                neighbor.pos = pos;
                neighbor.parentPos = current.pos;

                neighbor.g = current.g + 1;             // increase cost by one since its one step away from current
                neighbor.h = (                          // euclidian distance, skip sqrtf() since it doesn't actually change anything (I believe)
                    (pos.x - goal.x) * (pos.x - goal.x)
                  + (pos.y - goal.y) * (pos.y - goal.y)
                );
                // neighbor.h = sqrtf((pos.x - goal.x) * (pos.x - goal.x) + (pos.y - goal.y) * (pos.y - goal.y));

                neighbor.f = neighbor.g + neighbor.h;   // this ensures shortest path is found

                notVisitedMap[neighbor.pos] = neighbor;
                notVisitedHeap.push(neighbor);

                if (printDebugInfo) allocations++;
            }
        }

        printf("no path?\n");
        return std::list<Vector2i>();
    }
}
