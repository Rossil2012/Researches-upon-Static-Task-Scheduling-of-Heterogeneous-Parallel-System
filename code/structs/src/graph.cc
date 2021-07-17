#include "graph/graph.h"

void Node::addInNode(const NodePtr &in_node) {
    in_nodes_.push_back(NodeWeakPtr(in_node));
}

void Node::addOutNode(const NodePtr &out_node) {
    out_nodes_.push_back(NodeWeakPtr(out_node));
}