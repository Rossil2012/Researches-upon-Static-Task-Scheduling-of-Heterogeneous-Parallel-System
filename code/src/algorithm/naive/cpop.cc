#include "naive/cpop.h"

#include <unordered_set>

void CPOP::Schedule() {
    auto tl_list = ComputeTopLevel(task_graph_), bl_list = ComputeBottomLevel(task_graph_);
    decltype(tl_list) priority(tl_list.size());
    for (size_t i = 0 ; i < tl_list.size(); i++) {
        priority[i] = tl_list[i] + bl_list[i];
    }

    std::vector<TaskPtr> critical_path;

    auto cp_priority = priority[0];

    auto cur = task_graph_->GetTask(0);
    while (true) {
        if (cur->node_id == task_graph_->GetNodeNum() - 1) {
            break;
        }

        for (const auto &to_ : cur->out_nodes) {
            auto to = std::dynamic_pointer_cast<Task>(to_.lock());
            if (priority[to->node_id] == cp_priority) {
                cur = to;
                critical_path.push_back(std::move(to));
                break;
            }
        }
    }

    auto callback = [&critical_path](DevicePtr &device) {
        for (auto &cp_task : critical_path) {
            if (device->AvailMemory() < cp_task->MemorySize()) {

            } else {

            }
        }
    };

    device_graph_->Traverse(callback);
}

LogicalTime CPOP::GetExecTime() {
    return 0;
}
