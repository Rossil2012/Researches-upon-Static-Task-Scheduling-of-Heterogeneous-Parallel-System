#ifndef SCHEDULE_STRUCTS_TASK_GRAPH_H_
#define SCHEDULE_STRUCTS_TASK_GRAPH_H_

#include "graph/graph.h"

class TaskGraph;

using TaskGraphPtr = std::shared_ptr<TaskGraph>;
using TaskGraphWeakPtr = std::weak_ptr<TaskGraph>;

class TaskGraph : public Graph {

};

#endif //SCHEDULE_STRUCTS_TASK_GRAPH_H_
