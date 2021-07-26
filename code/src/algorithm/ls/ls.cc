#include "ls/ls.h"

void LS::Schedule() {
    genNodeList();
    genProcessorAllocation();
}

LogicalTime LS::GetExecTime() {
    LogicalTime max_exec_time = 0;

    auto callback = [&max_exec_time](DevicePtr &device) {
        max_exec_time = std::max(max_exec_time, EarliestAvailTimeOnDevice(device));
    };

    device_graph_->Traverse(callback);

    return max_exec_time;
}

LogicalTime LS::earliestTimeOnDevice(const TaskPtr &task, const DevicePtr &device, bool isFinish) {
    LogicalTime prepared_time = 0;  // time when all input data is ready
    for (const auto &node : task->in_nodes) {
        auto from = std::dynamic_pointer_cast<Task>(node.lock());
        auto from_device = device_graph_->GetDevice(from->allocated_to);
        auto task_record = from_device->GetRecordOfTask(from->node_id);
        // max(parents' finish_time + comm_time)
        prepared_time = std::max(prepared_time, task_record.time_slice.start_time + task_record.time_slice.exec_time
            + device_graph_->GetCommTimeBetweenDevices(from_device->node_id, device->node_id, from->output_size));
    }

    return std::max(prepared_time, EarliestAvailTimeOnDevice(device))
        + (isFinish ? device->GetCompTimeOnTask(task) : 0);
}

// minimal finish time
void LS::genProcessorAllocation() {
    processor_allocation_.resize(node_list_.size());
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