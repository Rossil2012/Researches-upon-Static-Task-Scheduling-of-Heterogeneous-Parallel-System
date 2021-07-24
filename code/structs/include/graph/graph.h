#ifndef SCHEDULE_STRUCTS_GRAPH_H_
#define SCHEDULE_STRUCTS_GRAPH_H_

#include <vector>

using NodeID = uint64_t;

class Node;

using NodePtr = std::shared_ptr<Node>;
using NodeWeakPtr = std::weak_ptr<Node>;

class Graph;

using GraphPtr = std::shared_ptr<Graph>;
using GraphWeakPtr = std::weak_ptr<Graph>;


class Node {
public:
    explicit Node() = default;
    Node(const Node &ano) = default;

    virtual void isBase() {};   // a hint for the ability of dynamic_pointer_cast

    inline void addInNode(const NodePtr &in_node) {
        in_nodes.push_back(NodeWeakPtr(in_node));
    }

    inline void addOutNode(const NodePtr &out_node) {
        out_nodes.push_back(NodeWeakPtr(out_node));
    }

    NodeID node_id = -1;
    std::vector<NodeWeakPtr> in_nodes, out_nodes; // all shared_ptr are held by graph
};


class Graph {
public:
    virtual inline NodeID NewNode() {
        auto node = std::make_shared<Node>();
        NodeID id = all_nodes_.size();
        node->node_id = id;
        all_nodes_.push_back(std::move(node));
        return id;
    }

    virtual inline void AddNode(NodePtr &&node) {
        all_nodes_.push_back(std::move(node));
    }

    virtual inline void AddEdge(NodeID from, NodeID to) {
        all_nodes_[from]->addOutNode(all_nodes_[to]);
        all_nodes_[to]->addInNode(all_nodes_[from]);
    }

    inline NodeID GetNodeNum() const {
        return all_nodes_.size();
    }

protected:
    std::vector<NodePtr> all_nodes_;
};


#endif //SCHEDULE_STRUCTS_GRAPH_H_
