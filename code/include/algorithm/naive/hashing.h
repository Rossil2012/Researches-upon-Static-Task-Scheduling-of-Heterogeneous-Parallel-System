#ifndef SCHEDULE_ALGORITHM_NAIVE_HASHING_H_
#define SCHEDULE_ALGORITHM_NAIVE_HASHING_H_

#include "ls/ls_pa.h"

class Hashing {
public:
    explicit Hashing(TaskGraphPtr task_graph, DeviceGraphPtr device_graph)
        : task_graph_(std::move(task_graph)),
          device_graph_(std::move(device_graph)) {}

    void Schedule();
    LogicalTime GetExecTime();

protected:
    TypePA hashTaskToDevice();

    TaskGraphPtr task_graph_;
    DeviceGraphPtr device_graph_;
    LS_PA *ls_pa_ = nullptr;
};

#endif //SCHEDULE_ALGORITHM_NAIVE_HASHING_H_
