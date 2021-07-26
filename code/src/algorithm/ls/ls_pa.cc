#include "ls/ls_pa.h"

void LS_PA::Schedule() {
    phaseProcessorAllocation();
    phaseNodeList();
}

void LS_PA::phaseProcessorAllocation() {}

void LS_PA::phaseNodeList() {
    auto callback = [this](TaskPtr &cur) {
        LogicalTime data_ready_time = 0;
        for (auto &from_ : cur->in_nodes) {
            auto from = std::dynamic_pointer_cast<Task>(from_.lock());
            auto from_device = device_graph_->GetDevice(processor_allocation_[from->node_id]);
            auto from_time_slice = from_device->GetRecordOfTask(from->node_id).time_slice;
            auto cur_ready_time = from_time_slice.start_time + from_time_slice.exec_time +
                    device_graph_->GetCommTimeBetweenDevices(processor_allocation_[from->node_id],
                                                             processor_allocation_[cur->node_id],
                                                             from->output_size);
            data_ready_time = std::max(data_ready_time, cur_ready_time);
        }

        auto cur_device = device_graph_->GetDevice(processor_allocation_[cur->node_id]);
        auto processor_avail_time = EarliestAvailTimeOnDevice(cur_device);
        cur_device->AddTaskToSchedule(cur, std::max(data_ready_time, processor_avail_time),
            cur_device->GetCompTimeOnTask(cur));
    };

    task_graph_->TraverseTopo(callback);
}
