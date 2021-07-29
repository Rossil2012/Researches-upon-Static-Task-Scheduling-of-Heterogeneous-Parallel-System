#include "ls/ls.h"

void LS::Schedule() {
    phaseNodeList();
    phaseProcessorAllocation();
}

LogicalTime LS::GetExecTime() {
    LogicalTime max_exec_time = 0;

    auto callback = [&max_exec_time](DevicePtr &device) {
        max_exec_time = std::max(max_exec_time, EarliestAvailTimeOnDevice(device));
    };

    device_graph_->Traverse(callback);

    return max_exec_time;
}

// minimal finish time
void LS::phaseProcessorAllocation() {
    processor_allocation_.resize(node_list_.size());
    for (auto &task : node_list_) {
        LogicalTime min_finish_time = -1;
        DeviceID min_device_ = 0;

        auto callback = [&](DevicePtr &device) {
            if (task->MemorySize() <= device->AvailMemory()) {   // device has enough memory
                LogicalTime finish_time = EarliestTimeOnDevice(device_graph_, task, device, true);
                if (finish_time < min_finish_time) {  // earliest finish time
                    min_finish_time = finish_time;
                    min_device_ = device->node_id;
                }
            }
        };

        device_graph_->Traverse(callback);

        if (min_finish_time == -1) {
            throw "LS: out of memory";
        }

        auto min_device = device_graph_->GetDevice(min_device_);
        LogicalTime exec_time = min_device->GetCompTimeOnTask(task);
        min_device->AddTaskToSchedule(task, min_finish_time - exec_time, exec_time);
        processor_allocation_[task->node_id] = min_device_;
    }
}