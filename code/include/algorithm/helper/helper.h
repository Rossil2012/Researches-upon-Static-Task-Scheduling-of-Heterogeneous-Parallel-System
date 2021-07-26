#ifndef SCHEDULE_ALGORITHM_HELPER_H_
#define SCHEDULE_ALGORITHM_HELPER_H_

#include <algorithm>
#include <queue>
#include <random>

#include "global.h"
#include "device/device.h"
#include "device/device_graph.h"
#include "task/task.h"
#include "task/task_graph.h"


struct ScheduleResult {
    std::vector<DeviceID> allocation;
    LogicalTime exec_time;
};

ScheduleResult GetExecResultWithPA(const TaskGraphPtr &task_graph,
                                   const DeviceGraphPtr &device_graph,
                                   const std::vector<DeviceID> &allocation);

std::vector<size_t> ComputeTopLevel(const TaskGraphPtr &task_graph);
std::vector<size_t> ComputeBottomLevel(const TaskGraphPtr &task_graph);

bool RandomWithProbability(double prob);

std::vector<TaskPtr> CreateNodeListFromPriority(const TaskGraphPtr &task_graph, const std::vector<size_t> &priority);

template <typename T>
T RandomWithRange(T low, T high) {  // [low, high)
    std::uniform_int_distribution<T> u(low, high-1);
    std::default_random_engine e(g_random_device());
    return u(e);
}

template <typename T>
void ShuffleVector(std::vector<T> &vec) {
    std::default_random_engine e(g_random_device());
    std::shuffle(vec.begin(), vec.end(), e);
}


// NOTE: PopVector would change the order of vector
template <typename T>
void PopVector(T &to_pop, std::vector<T> &vec) {
    std::swap(to_pop, vec.back());
    vec.pop_back();
}

template <typename T>
void AppendVectorWithMove(std::vector<T> &vec, std::vector<T> &&to_append) {
    vec.reserve(vec.size() + to_append.size());
    vec.insert(vec.end(), std::make_move_iterator(to_append.begin()), std::make_move_iterator(to_append.end()));
//    to_append.swap(std::vector<int>());
}

template <typename F, typename T>
T &RouletteWheel(std::vector<T> &vec, F tot_weight, const std::function<F(T&)> &get_weight) {
    F rand_low = 0, rand_high = tot_weight;

    F rand_weight = RandomWithRange(rand_low, rand_high);
    F acc_weight = 0;
    for (auto &item : vec) {
        if ((acc_weight += get_weight(item)) > rand_weight) {
            return item;
        }
    }

    auto r = RandomWithRange(size_t(0), vec.size());
    return vec[r];
}

LogicalTime EarliestAvailTimeOnDevice(const DevicePtr &device);

template<typename T>
std::vector<T> GenIncPriorVector(size_t n) {
    std::vector<T> vec; vec.reserve(n);
    for (T i = 0; i < n; i++) { vec.push_back(i); }
    return std::move(vec);
}

#endif //SCHEDULE_ALGORITHM_HELPER_H_
