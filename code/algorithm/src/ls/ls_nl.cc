#include "ls/ls_nl.h"

// no use
void LS_NL::genNodeList() {}

// minimal finish time
void LS_NL::genProcessorAllocation() {
    for (auto &task : node_list_) {
        LogicalTime min_finish_time = -1;
        DeviceID min_device_ = processor_allocation_[0];

        auto callback = [&](DevicePtr &device) {
            if (task->MemorySize() <= device->AvailMemory()) {   // device has enough memory
                LogicalTime finish_time = earliestTimeOnDevice(task, device, true);
                if (finish_time < min_finish_time) {  // earliest finish time
                    min_finish_time = finish_time;
                    min_device_ = device->node_id;
                }
            }
        };

        device_graph_->Traverse(callback);

        if (min_finish_time == -1) {
            throw "Device: lack of memory";
        }

        auto min_device = device_graph_->GetDevice(min_device_);
        LogicalTime exec_time = min_device->GetCompTimeOnTask(task);
        min_device->AddTaskToSchedule(task, min_finish_time - exec_time, exec_time);
        processor_allocation_[task->node_id] = min_device_;
    }
}
