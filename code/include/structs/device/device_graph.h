#ifndef SCHEDULE_STRUCTS_DEVICE_DEVICE_H_
#define SCHEDULE_STRUCTS_DEVICE_DEVICE_H_

#include "global.h"
#include "graph/graph.h"

class DeviceGraph;

using DeviceGraphPtr = std::shared_ptr<DeviceGraph>;
using DeviceGraphWeakPtr = std::weak_ptr<DeviceGraph>;

class DeviceGraph : public Graph {
public:
    inline DevicePtr GetDevice(DeviceID device_id) {
        return std::dynamic_pointer_cast<Device>(all_nodes_[device_id]);
    }

    void Traverse(const std::function<void(DevicePtr &)> &callback);

    inline void AddEdge(DeviceID a_, NodeID b_, size_t bandwidth) {
        auto &a = all_nodes_[a_], &b = all_nodes_[b_];
        (*bandwidth_map_)[a_ * node_num_ + b_] = (*bandwidth_map_)[b_ * node_num_ + a_] = bandwidth;
        a->addInNode(b); a->addOutNode(b);
        b->addInNode(a); b->addOutNode(a);
    }

    inline DeviceID NewCPU(CPU::SIMD support_isa, size_t memory_constraint, double frequency, size_t num_core) {
        auto cpu = DevicePtr(new CPU(support_isa, memory_constraint, frequency, num_core));
        DeviceID id = all_nodes_.size();
        cpu->node_id = id;
        all_nodes_.push_back(std::move(cpu));
        return id;
    };

    inline DeviceID NewGPU(size_t memory_constraint, double frequency, size_t num_cuda) {
        auto gpu = DevicePtr(new GPU(memory_constraint, frequency, num_cuda));
        DeviceID id = all_nodes_.size();
        gpu->node_id = id;
        all_nodes_.push_back(std::move(gpu));
        return id;
    }

    inline void NewNodeFinished() {
        node_num_ = all_nodes_.size();
        bandwidth_map_ = new std::vector<size_t>(node_num_ * node_num_, 0);
    }

    LogicalTime GetCommTimeBetweenDevices(DeviceID from, DeviceID to, size_t data_size) const;

    DeviceGraphPtr Clone() const;

protected:
    size_t node_num_ = 0;
    std::vector<size_t> *bandwidth_map_ = nullptr;  // unit of bandwidth: kb/s
};

#endif //SCHEDULE_STRUCTS_DEVICE_DEVICE_H_
