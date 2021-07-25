#include "device/device.h"
#include "device/device_graph.h"


DeviceGraphPtr DeviceGraph::Clone() const {
    auto new_graph = std::make_shared<DeviceGraph>();
    new_graph->node_num_ = this->node_num_;
    new_graph->bandwidth_map_ = this->bandwidth_map_;
    new_graph->all_nodes_.reserve(all_nodes_.size());

    for (const auto &node : all_nodes_) {
        auto new_device = std::dynamic_pointer_cast<Device>(node)->Clone();
        new_graph->all_nodes_.push_back(std::move(new_device));
    }

    for (auto &new_node : new_graph->all_nodes_) {  // the new_nodes still point to old_tasks' pointers
        for (auto &old_out : new_node->out_nodes) {
            old_out = NodeWeakPtr(new_graph->all_nodes_[old_out.lock()->node_id]);  // point to new_nodes
        }

        for (auto &old_in : new_node->in_nodes) {
            old_in = NodeWeakPtr(new_graph->all_nodes_[old_in.lock()->node_id]);  // point to new_nodes
        }
    }

    return new_graph;
}

void DeviceGraph::Traverse(const std::function<void(DevicePtr &)> &callback) {
    for (auto &node : all_nodes_) {
        auto device = std::dynamic_pointer_cast<Device>(node);
        callback(device);
    }
}

LogicalTime DeviceGraph::GetCommTimeBetweenDevices(DeviceID from, DeviceID to, size_t data_size) const {
    constexpr static double const_contention = 0.01; // 10ms contention
    constexpr static LogicalTime LT_per_second = 1000000000 * LogicalTimeGain;
    auto bandwidth = double(bandwidth_map_->at(from * node_num_ + to));
    auto ret = bandwidth == 0 ? 0 :
            LogicalTime((double(data_size) / (bandwidth * 1000) + const_contention) * LT_per_second);
    return ret;
}

LogicalTime CPU::GetCompTimeOnTask(const TaskPtr &task) const {
    LogicalTime comp_time = 0;
    for (const auto &tasklet : task->task_flow) {
        comp_time += (tasklet.data_size / process_cap[tasklet.data_type] / num_core_ + 1) *
                LogicalTime(ns_per_cycle_ * LogicalTimeGain);
    }
    return comp_time;
}

DevicePtr CPU::Clone() const {
    auto new_cpu = DevicePtr(new CPU(*this));
    new_cpu->ResetTemp();
    return std::move(new_cpu);
}

LogicalTime GPU::GetCompTimeOnTask(const TaskPtr &task) const {
    LogicalTime comp_time = 0;
    for (const auto &tasklet : task->task_flow) {
        comp_time += (tasklet.data_size / num_cuda_ + 1) *
                     LogicalTime(ns_per_cycle_ * LogicalTimeGain);
    }
    return comp_time;
}

DevicePtr GPU::Clone() const {
    auto new_gpu = DevicePtr(new GPU(*this));
    new_gpu->ResetTemp();
    return std::move(new_gpu);
}

LogicalTime Ascend::GetCompTimeOnTask(const TaskPtr &task) const {
    return 0;
}

DevicePtr Ascend::Clone() const {
    auto new_ascend = DevicePtr(new Ascend(*this));
    new_ascend->ResetTemp();
    return std::move(new_ascend);
}
