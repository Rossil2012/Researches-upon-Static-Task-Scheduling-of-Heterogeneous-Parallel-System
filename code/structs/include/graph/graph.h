#ifndef SCHEDULE_STRUCTS_GRAPH_H_
#define SCHEDULE_STRUCTS_GRAPH_H_

#include <vector>

class Node;

using NodePtr = std::shared_ptr<Node>;
using NodeWeakPtr = std::weak_ptr<Node>;

class Graph;

using GraphPtr = std::shared_ptr<Graph>;
using GraphWeakPtr = std::weak_ptr<Graph>;


class Node {
public:
    explicit Node(const GraphPtr &graph)
            : graph_(GraphWeakPtr(graph)) {}
    void addInNode(const NodePtr &in_node);
    void addOutNode(const NodePtr &out_node);
protected:
    GraphWeakPtr graph_;
    std::vector<NodeWeakPtr> in_nodes_, out_nodes_; // all shared_ptr are held by Graph
};


class Graph {
public:
    explicit Graph() = default;
protected:
    std::vector<NodePtr> all_nodes_;
};


#endif //SCHEDULE_STRUCTS_GRAPH_H_
