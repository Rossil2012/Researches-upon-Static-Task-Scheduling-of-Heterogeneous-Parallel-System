#include "helper/helper.h"


static LogicalTime CalculateMaxExecTimeFromTask(const TaskPtr &from,
                                                const DeviceGraphPtr &device_graph,
                                                const std::vector<DeviceID> &allocation,
                                                std::vector<LogicalTime> &max_exec_time) {
    LogicalTime ret;
    if ((ret = max_exec_time[from->node_id]) > 0) {
        return ret;
    }

    auto from_device = device_graph->GetDevice(allocation[from->node_id]);
    LogicalTime comp_time = from_device->GetCompTimeOnTask(from);
    for (const auto &to_ : from->out_nodes) {
        auto to = std::dynamic_pointer_cast<Task>(to_.lock());
        auto to_device = device_graph->GetDevice(allocation[to->node_id]);
        LogicalTime comm_time = from_device->GetCommTimeWithDevice(to_device, from->output_size);
        ret = std::max(ret, CalculateMaxExecTimeFromTask(to, device_graph, allocation, max_exec_time)
                                + comp_time + comm_time);
    }

    max_exec_time[from->node_id] = ret;
    return ret;
}

ScheduleResult GetExecResultWithPA(const TaskGraphPtr &task_graph,
                                   const DeviceGraphPtr &device_graph,
                                   const std::vector<DeviceID> &allocation) {
    LogicalTime exec_time = 0;
    std::vector<TaskPtr> topo_tasks;
    std::vector<LogicalTime> max_exec_time;
    auto callback = [&topo_tasks, &max_exec_time](const TaskPtr &cur) {
        topo_tasks.push_back(cur);
        max_exec_time.push_back(0);
    };
    task_graph->TraverseTopo(callback);

    for (auto &task : topo_tasks) {
        exec_time = std::max(exec_time, CalculateMaxExecTimeFromTask(task, device_graph, allocation, max_exec_time));
    }

    return {
        .allocation = allocation,
        .exec_time = exec_time,
    };
}

bool RandomWithProbability(double prob) {
    static std::random_device dev;
    static std::uniform_real_distribution<double> u(0, 1);
    static std::default_random_engine e(dev());
    return u(e) <= prob;
}

std::vector<TaskPtr> CreateNodeListFromPriority(const TaskGraphPtr &task_graph, const std::vector<size_t> &priority) {
    std::vector<TaskPtr> ret;

    ret.reserve(task_graph->GetNodeNum());
    auto callback = [&ret](TaskPtr &task) {
        ret.push_back(task);
    };
    task_graph->TraversePriorTopo(callback, priority);

    return std::move(ret);
}

LogicalTime EarliestAvailTimeOnDevice(const DevicePtr &device) {
    auto schedule = device->CurrentSchedule();
    auto latest_task = schedule.rbegin();
    return schedule.empty() ? 0 : latest_task->first + latest_task->second;
}
