#include "ls/ls_asap.h"

void LS_ASAP::phaseNodeList() {
    node_list_ = CreateNodeListFromPriority(task_graph_, ComputeTopLevel(task_graph_));
}
