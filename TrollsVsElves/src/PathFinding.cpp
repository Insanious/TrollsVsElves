#include "PathFinding.h"

#include "time.h" // time_t
#include <iomanip>
#include <iostream>
#include <limits>

PathFinding::PathFinding() {}

void PathFinding::printNode(std::string prefix, Node node)
{
    printf("%s: x: %d, y: %d\tg: %f, h: %f, f: %f\n", prefix.c_str(), node.position.x, node.position.y, node.g, node.h, node.f);
}

double PathFinding::weightedConvexUpwardParabola(double g, double h)
{
    double weight = 2.f;
    double numerator = g + h + sqrt(pow(g + h, 2) + 4 * weight * (weight - 1) * pow(h, 2));
    double denominator = 2 * weight;
    return numerator / denominator;
}

double PathFinding::weightedConvexDownwardParabola(double g, double h)
{
    double weight = 2.f;
    double numerator = g + (2 * weight - 1) * h + sqrt(pow(g - h, 2) + 4 * weight * g * h);
    double denominator = 2 * weight;
    return numerator / denominator;
}

std::list<Vector2i> backtrack(std::unordered_map<Vector2i, Node, HashVector2i> nodes, Node start, Node node)
{
    std::list<Vector2i> path;
    while (node != start)
    {
        path.push_front(node.position);
        node = nodes[node.parentPosition];
    }
    return path;
}

std::list<Vector2i> PathFinding::findPath(Vector2i start, Vector2i goal, const std::vector<std::vector<bool>> obstacles)
{
    clock_t start_t = clock();      // for debugging purposes
    int allocations = 0;            // for debugging purposes
    bool printDebugInfo = false;    // for debugging purposes

    printNode("start", start);
    printNode("goal", goal);

    int maxY = obstacles.size();
    int maxX = obstacles[0].size();
    Node current, neighbor;
    Node startNode = Node(start);
    Node goalNode = Node(goal);
    Vector2i neighborPositions[8] = { { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 } };
    const int directions[8][2] = {
        {-1,  0},
        { 1,  0},
        { 0, -1},
        { 0,  1},

        {-1, -1}, // diagonals
        { 1,  1}, // diagonals
        {-1,  1}, // diagonals
        { 1, -1}, // diagonals
    };

    boost::heap::fibonacci_heap<Node, boost::heap::compare<CompareNode>> notVisitedHeap;
    std::unordered_map<Vector2i, Node, HashVector2i> notVisitedMap;
    std::unordered_map<Vector2i, Node, HashVector2i> visited;
    std::list<Vector2i> path;

    notVisitedHeap.push(startNode);
    while (!notVisitedHeap.empty())
    {
        current = notVisitedHeap.top();
        notVisitedHeap.pop();

        visited[current.position] = current;

        if (current == goalNode)
        {
            if (printDebugInfo)
                printf("time, allocations, visited: %f, %d, %d\n", double(clock()-start_t)/CLOCKS_PER_SEC, allocations, visited.size());

            return backtrack(visited, startNode, current);
        }

        for (int i = 0; i < 8; ++i) // calculate new positions
        {
            neighborPositions[i].x = current.position.x + directions[i][0];
            neighborPositions[i].y = current.position.y + directions[i][1];
        }

        for (Vector2i pos : neighborPositions)
        {
            if (pos.x < 0 || pos.x >= maxX || pos.y < 0 || pos.y >= maxY    // out of bounds
                || visited.find(pos) != visited.end()                       // already visited
                || notVisitedMap.find(pos) != notVisitedMap.end()           // already in queue
                || obstacles[pos.y][pos.x])                                 // is not traversable
                continue;

            neighbor.position = pos;
            neighbor.parentPosition = current.position;

            neighbor.g = current.g + 1;             // increase cost by one since its one step away from current
            neighbor.h = sqrtf(                     // euclidian distance
                (pos.x - goal.x) * (pos.x - goal.x)
              + (pos.y - goal.y) * (pos.y - goal.y)
            );
            neighbor.f = neighbor.g + neighbor.h;   // this ensures shortest path is found

            notVisitedMap[neighbor.position] = neighbor;
            notVisitedHeap.push(neighbor);

            if (printDebugInfo) allocations++;
        }
    }

    printf("no path?\n");

    return path;
}
