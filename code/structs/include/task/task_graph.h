#ifndef SCHEDULE_STRUCTS_TASK_GRAPH_H_
#define SCHEDULE_STRUCTS_TASK_GRAPH_H_

#include "graph/graph.h"
#include "task.h"


class TaskGraph;

using TaskGraphPtr = std::shared_ptr<TaskGraph>;
using TaskGraphWeakPtr = std::weak_ptr<TaskGraph>;

class TaskGraph : public Graph {
public:
    inline TaskPtr GetTask(TaskID task_id) {
        return std::dynamic_pointer_cast<Task>(all_nodes_[task_id]);
    }

    inline TaskID NewNode() override {
        auto task = std::make_shared<Task>();
        TaskID id = all_nodes_.size();
        task->node_id = id;
        all_nodes_.push_back(std::move(task));
        return id;
    }

    inline TaskID NewTask(size_t output_size, std::vector<Tasklet> &&task_flow) {
        auto task = TaskPtr(new Task(output_size, std::move(task_flow)));
        TaskID id = all_nodes_.size();
        task->node_id = id;
        all_nodes_.push_back(std::move(task));
        return id;
    }

    TaskGraphPtr Clone();

    void TraverseTopo(const std::function<void(TaskPtr &)> &callback);

    void TraversePriorTopo(const std::function<void(TaskPtr &)> &callback,
                           const std::vector<size_t> &priority);

protected:
    std::vector<TaskPtr> getAllSources();
};

#endif //SCHEDULE_STRUCTS_TASK_GRAPH_H_
