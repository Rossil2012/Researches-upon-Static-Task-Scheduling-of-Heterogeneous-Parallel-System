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
        prepared_time = std::max(prepared_time, task_record.time_slice.start_time + task_record.time_slice.exec_time
            + from_device->GetCommTimeWithDevice(device, from->output_size));   // max(parents' finish_time + comm_time)
    }

    return std::max(prepared_time, EarliestAvailTimeOnDevice(device))
        + (isFinish ? device->GetCompTimeOnTask(task) : 0);
}
