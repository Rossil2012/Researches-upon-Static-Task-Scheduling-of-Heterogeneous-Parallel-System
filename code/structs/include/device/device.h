#ifndef SCHEDULE_STRUCTS_DEVICE_H_
#define SCHEDULE_STRUCTS_DEVICE_H_

#include "structs/include/graph/graph.h"
#include "structs/include/task/task.h"

class Device;

using DevicePtr = std::shared_ptr<Device>;
using DeviceWeakPtr = std::weak_ptr<Device>;

class Device : public Node {
public:
    virtual LogicalTime GetCompTimeOnTask(const Task &task) = 0;
};

class GPU : public Device {
public:
    LogicalTime GetCompTimeOnTask(const Task &task) override;
};

class CPU : public Device {
public:
    LogicalTime GetCompTimeOnTask(const Task &task) override;
};

class Ascend : public Device {
public:
    LogicalTime GetCompTimeOnTask(const Task &task) override;
};

#endif //SCHEDULE_STRUCTS_DEVICE_H_
