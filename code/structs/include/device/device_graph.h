#ifndef SCHEDULE_STRUCTS_DEVICE_DEVICE_H_
#define SCHEDULE_STRUCTS_DEVICE_DEVICE_H_

#include "graph/graph.h"

class DeviceGraph;

using DeviceGraphPtr = std::shared_ptr<DeviceGraph>;
using DeviceGraphWeakPtr = std::weak_ptr<DeviceGraph>;

#endif //SCHEDULE_STRUCTS_DEVICE_DEVICE_H_
