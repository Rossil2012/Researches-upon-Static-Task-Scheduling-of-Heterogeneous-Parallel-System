#ifndef SCHEDULE_ALGORITHM_NAIVE_CPOP_H_
#define SCHEDULE_ALGORITHM_NAIVE_CPOP_H_

#include "device/device.h"
#include "device/device_graph.h"
#include "helper/helper.h"
#include "ls/ls_pa.h"
#include "task/task.h"
#include "task/task_graph.h"

class CPOP {
public:
    explicit CPOP(TaskGraphPtr task_graph, DeviceGraphPtr device_graph)
        : task_graph_(std::move(task_graph)),
          device_graph_(std::move(device_graph)) {}

    void Schedule();
    LogicalTime GetExecTime();

protected:
    LogicalTime getMinimumPathAllocation(const std::vector<DeviceID> &avail_devices, DeviceID from_device,
                                         const std::vector<TaskPtr> &cp, size_t start, TypePA *processor_allocation);

    TaskGraphPtr task_graph_;
    DeviceGraphPtr device_graph_;
};

#endif //SCHEDULE_ALGORITHM_NAIVE_CPOP_H_
