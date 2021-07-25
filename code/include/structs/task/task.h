#ifndef SCHEDULE_STRUCTS_TASK_NODE_H_
#define SCHEDULE_STRUCTS_TASK_NODE_H_

#include "global.h"
#include "graph/graph.h"

using LogicalTime = uint64_t;
using TaskID = uint64_t;
using DeviceID = uint64_t; // TODO: duplicated declaration of DeviceID

class Task;

using TaskPtr = std::shared_ptr<Task>;
using TaskWeakPtr = std::weak_ptr<Task>;

enum DataType {
    INT32 = 0, INT64, FLOAT, DOUBLE
};

struct Tasklet {
    DataType data_type;
    size_t data_size;
};

class Task : public Node {
    friend class TaskGraph;
public:
    explicit Task()
        : param_size(0),
          output_size(0) {}

    explicit Task(size_t output_size, std::vector<Tasklet> &&task_flow)
        : output_size(output_size),
          task_flow(std::move(task_flow)) {
        param_size = getParamSizeFromTaskFlow(this->task_flow);
    }

    Task(const Task &ano) = default;

    inline size_t MemorySize() const {
        return input_size + param_size + output_size;
    }

    inline void ResetTemp() {
        in_graph = true;
        in_degree = in_nodes.size();
        out_degree = out_nodes.size();
    }

    TaskPtr Clone() const;

    size_t input_size = 0;
    size_t param_size = 0;
    size_t output_size = 0;

    std::vector<Tasklet> task_flow; // a task is abstracted as a serial of parallel sub_tasks

    bool in_graph = false;
    size_t in_degree = 0, out_degree = 0;   // temporary variables

    DeviceID allocated_to = -1;

protected:
    inline static size_t getParamSizeFromTaskFlow(const std::vector<Tasklet> &flow) {
        size_t ret = 0;
        for (const auto &tasklet : flow) {
            ret += (tasklet.data_type == INT32 || tasklet.data_type == FLOAT ? 4 : 8);
        }
        return ret;
    }
};

#endif //SCHEDULE_STRUCTS_TASK_NODE_H_
