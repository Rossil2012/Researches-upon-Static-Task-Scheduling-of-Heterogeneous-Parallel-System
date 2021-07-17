#include "device/device.h"
#include "device/device_graph.h"

LogicalTime GPU::GetCompTimeOnTask(const Task &task) {
    return 0;
}

LogicalTime CPU::GetCompTimeOnTask(const Task &task) {
    return 0;
}

LogicalTime Ascend::GetCompTimeOnTask(const Task &task) {
    return 0;
}
