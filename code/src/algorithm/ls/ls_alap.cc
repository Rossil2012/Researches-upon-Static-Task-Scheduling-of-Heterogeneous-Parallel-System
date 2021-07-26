#include "ls/ls_alap.h"

void LS_ALAP::genNodeList() {
    auto bl_list = ComputeBottomLevel(task_graph_);
    auto max_prior = *std::max_element(bl_list.begin(), bl_list.end());
    for (auto &priority : bl_list) {
        priority = max_prior - priority;
    }
    node_list_ = CreateNodeListFromPriority(task_graph_, bl_list);
}