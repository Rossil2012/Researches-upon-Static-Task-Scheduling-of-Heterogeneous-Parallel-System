#ifndef SCHEDULE_STRUCTS_TASK_NODE_H_
#define SCHEDULE_STRUCTS_TASK_NODE_H_

#include "graph/graph.h"

using LogicalTime = uint64_t;

class Task;

using TaskPtr = std::shared_ptr<Task>;
using TaskWeakPtr = std::weak_ptr<Task>;

class Task : public Node {

};

#endif //SCHEDULE_STRUCTS_TASK_NODE_H_
