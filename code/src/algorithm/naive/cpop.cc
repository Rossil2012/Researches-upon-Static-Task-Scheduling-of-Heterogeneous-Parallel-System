#include "naive/cpop.h"

#include <unordered_set>

void CPOP::Schedule() {
    // compute node level (bottom + top)
    auto tl_list = ComputeTopLevel(task_graph_), bl_list = ComputeBottomLevel(task_graph_);
    decltype(tl_list) priority(tl_list.size());
    for (size_t i = 0 ; i < tl_list.size(); i++) {
        priority[i] = tl_list[i] + bl_list[i];
    }

    std::vector<TaskPtr> critical_path;

    // the given task graph guarantees the first element is the unique source and the last is the unique sink
    auto cp_priority = priority[0];

    // compute critical path
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

    // get all devices
    std::vector<DeviceID> all_devices;
    all_devices.reserve(device_graph_->GetNodeNum());
    auto get_devices_callback = [&all_devices](DevicePtr &device) {
        all_devices.push_back(device->node_id);
    };
    device_graph_->Traverse(get_devices_callback);

    // recursively assign tasks in cp to devices to minimize cp execution, with memory constraint
    TypePA processor_allocation(task_graph_->GetNodeNum());
    getMinimumPathAllocation(all_devices, 0, critical_path, 0, &processor_allocation);

    auto task_callback = [&processor_allocation, &priority, cp_priority, this](TaskPtr &task) {
        if (priority[task->node_id] == cp_priority) { // tasks that have been allocated on the critical path
            auto cp_device = device_graph_->GetDevice(processor_allocation[task->node_id]);
            LogicalTime start_time = EarliestTimeOnDevice(device_graph_, task, cp_device, false);
            cp_device->AddTaskToSchedule(task, start_time, cp_device->GetCompTimeOnTask(task));
        } else {
            LogicalTime min_finish_time = -1;
            DeviceID min_device_ = 0;
            auto device_traverse_callback = [&task, &min_finish_time, &min_device_, this](DevicePtr &device) {
                if (task->MemorySize() <= device->AvailMemory()) {   // device has enough memory
                    LogicalTime finish_time = EarliestTimeOnDevice(device_graph_, task, device, true);
                    if (finish_time < min_finish_time) {  // earliest finish time
                        min_finish_time = finish_time;
                        min_device_ = device->node_id;
                    }
                }
            };
            device_graph_->Traverse(device_traverse_callback);
            if (min_finish_time == -1) {
                throw "LS: out of memory";
            }

            auto min_device = device_graph_->GetDevice(min_device_);
            LogicalTime exec_time = min_device->GetCompTimeOnTask(task);
            min_device->AddTaskToSchedule(task, min_finish_time - exec_time, exec_time);
            processor_allocation[task->node_id] = min_device_;
        }
    };

    task_graph_->TraversePriorTopo(task_callback, priority);
}

// a optimal algorithm using dynamic programming of scheduling a path to several devices
LogicalTime CPOP::getMinimumPathAllocation(const std::vector<DeviceID> &avail_devices, DeviceID from_device,
                                           const std::vector<TaskPtr> &cp, size_t start, TypePA *processor_allocation) {
    LogicalTime min_exec_time = -1;

    if (avail_devices.empty()) {
        throw "CPOP: out of memory";
    }

    for (auto cur_device_ = avail_devices.begin(); cur_device_ != avail_devices.end(); cur_device_++) {
        auto cur_device = device_graph_->GetDevice(*cur_device_)->Clone();
        auto cur_task_ = cp.cbegin() + start;
        auto cur_processor_allocation = new TypePA(*processor_allocation);
        LogicalTime cur_exec_time = start == 0 ? 0 : device_graph_->GetCommTimeBetweenDevices(from_device, *cur_device_,
            (*(cur_task_ - 1))->output_size);

        while (true) {
            if (cur_task_ == cp.cend()) {
                if (cur_exec_time < min_exec_time) {
                    min_exec_time = cur_exec_time;
                    processor_allocation = cur_processor_allocation;
                }
                break;
            }

            if (cur_device->AvailMemory() < (*cur_task_)->MemorySize()) {
                std::vector<DeviceID> next_avail_devices = avail_devices;
                next_avail_devices.erase(next_avail_devices.begin() + (cur_device_ - avail_devices.cbegin()));
                LogicalTime next_exec_time =
                        getMinimumPathAllocation(next_avail_devices, *cur_device_,
                                                 cp, cur_task_ - cp.cbegin(), cur_processor_allocation);
                if ((cur_exec_time += next_exec_time) < min_exec_time) {
                    min_exec_time = cur_exec_time;
                    processor_allocation = cur_processor_allocation;
                } else {
                    delete cur_processor_allocation;
                }
                break;
            }

            (*cur_processor_allocation)[(*cur_task_)->node_id] = cur_device->node_id;
            cur_exec_time += cur_device->GetCompTimeOnTask((*cur_task_));
            cur_device->AvailMemory() -= (*cur_task_)->MemorySize();
            cur_task_++;
        }
    }

    return min_exec_time;
}


LogicalTime CPOP::GetExecTime() {
    LogicalTime max_exec_time = 0;

    auto callback = [&max_exec_time](DevicePtr &device) {
        max_exec_time = std::max(max_exec_time, EarliestAvailTimeOnDevice(device));
    };

    device_graph_->Traverse(callback);

    return max_exec_time;
}
