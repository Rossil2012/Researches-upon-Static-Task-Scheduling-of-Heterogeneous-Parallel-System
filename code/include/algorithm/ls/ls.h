#ifndef SCHEDULE_ALGORITHM_LS_H_
#define SCHEDULE_ALGORITHM_LS_H_

#include "device/device.h"
#include "device/device_graph.h"
#include "helper/helper.h"
#include "task/task.h"
#include "task/task_graph.h"

using TypeNL = std::vector<TaskPtr>;
using TypePA = std::vector<DeviceID>;

class LS {
public:
    explicit LS(TaskGraphPtr &&task_graph, DeviceGraphPtr &&device_graph)
        : task_graph_(std::move(task_graph)),
          device_graph_(std::move(device_graph)) {}

    void Schedule();
    LogicalTime GetExecTime();
    inline TypeNL GetNodeList() {
        return node_list_;
    }
    inline TypePA GetProcessorAllocation() {
        return processor_allocation_;
    }
protected:
    TypeNL node_list_;              // node_list_[order] = TaskPtr
    TypePA processor_allocation_;   // processor_allocation_[TaskID] = DeviceID
    TaskGraphPtr task_graph_;
    DeviceGraphPtr device_graph_;

    virtual void phaseNodeList() {}
    virtual void phaseProcessorAllocation();

    LogicalTime earliestTimeOnDevice(const TaskPtr &task, const DevicePtr &device, bool isFinish);
};

#endif //SCHEDULE_ALGORITHM_LS_H_