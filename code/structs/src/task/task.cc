#include "task/task.h"
#include "task/task_graph.h"

#include <queue>

void TaskGraph::TraverseTopo(const std::function<void(TaskPtr &)> &callback) {
    auto all_sources = getAllSources();
    std::queue<TaskPtr> to_traverse;

    for (auto &source : all_sources) {
        to_traverse.push(std::move(source));
    }

    while (!to_traverse.empty()) {
        auto cur_node = to_traverse.front();
        to_traverse.pop();

        callback(cur_node);
        for (auto &to_ : cur_node->out_nodes) {
            auto to = std::dynamic_pointer_cast<Task>(to_.lock());
            if (--to->in_degree == 0) {
                to_traverse.push(std::move(to));
            }
        }
    }
}

class topoPriorityQueue {
public:
    explicit topoPriorityQueue(std::vector<TaskPtr> &&all_sources, const std::vector<size_t> &priority)
            : heap_(std::move(all_sources)) {
        priorityComp_ = [&priority](const TaskPtr &lhs, const TaskPtr &rhs) {
            return priority[lhs->node_id] < priority[rhs->node_id];
        };

        std::make_heap(heap_.begin(), heap_.end(), priorityComp_);
    }

    void push(TaskPtr &&node) {
        heap_.push_back(node);
        std::push_heap(heap_.begin(), heap_.end(), priorityComp_);
    }

    TaskPtr pop() {
        auto ret = heap_.front();
        std::pop_heap(heap_.begin(), heap_.end(), priorityComp_);
        heap_.pop_back();
        return std::move(ret);
    }

    inline bool empty() const {
        return heap_.empty();
    }

private:
    std::vector<TaskPtr> heap_;
    std::function<bool(const TaskPtr &, const TaskPtr &)> priorityComp_;
};

void TaskGraph::TraversePriorTopo(const std::function<void(TaskPtr &)> &callback,
                                  const std::vector<size_t> &priority) {

    topoPriorityQueue to_traverse(getAllSources(), priority);

    while (!to_traverse.empty()) {
        auto cur = to_traverse.pop();
        callback(cur);

        for (auto &to_ : cur->out_nodes) {
            auto to = std::dynamic_pointer_cast<Task>(to_.lock());
            if (--to->in_degree == 0) {
                to_traverse.push(std::move(to));
            }
        }
    }
}

std::vector<TaskPtr> TaskGraph::getAllSources() {
    std::vector<TaskPtr> ret;

    for (auto &node : all_nodes_) {
        auto task = std::dynamic_pointer_cast<Task>(node);
        task->ResetTemp();
        if (node->in_nodes.empty()) {
            ret.push_back(std::move(task));
        }
    }

    return std::move(ret);
}

TaskGraphPtr TaskGraph::Clone() {
    auto new_graph = std::make_shared<TaskGraph>();
    new_graph->all_nodes_.reserve(all_nodes_.size());

    for (const auto &old_node : all_nodes_) {
        auto new_task = std::dynamic_pointer_cast<Task>(old_node)->Clone();
        new_graph->all_nodes_.push_back(std::move(new_task));
    }

    for (auto &new_node : new_graph->all_nodes_) {  // the new_nodes still point to old_tasks' pointers
        for (auto &old_out : new_node->out_nodes) {
            old_out = NodeWeakPtr(new_graph->all_nodes_[old_out.lock()->node_id]);  // point to new_nodes
        }
    }

    return new_graph;
}

TaskPtr Task::Clone() const {
    auto new_task = TaskPtr(new Task(*this));
    new_task->ResetTemp();
    return std::move(new_task);
}
