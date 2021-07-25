#ifndef SCHEDULE_STRUCTS_DEVICE_H_
#define SCHEDULE_STRUCTS_DEVICE_H_

#include <map>
#include <unordered_map>

#include "global.h"
#include "graph/graph.h"
#include "task/task.h"

constexpr static LogicalTime LogicalTimeGain = 100; // which means 1 ns = 100 LogicalTime

using DeviceID = uint64_t;

class Device;

using DevicePtr = std::shared_ptr<Device>;
using DeviceWeakPtr = std::weak_ptr<Device>;

struct timeSlice {
    LogicalTime start_time;
    LogicalTime exec_time;
};

struct taskRecord {
    explicit taskRecord() = default;
    explicit taskRecord(LogicalTime start_time, LogicalTime exec_time, size_t memory_size)
        : time_slice{start_time, exec_time},
          memory_size(memory_size) {}

    timeSlice time_slice;
    size_t memory_size;
};

class Device : public Node {
public:
    explicit Device()
        : tot_memory_(0),
          avail_memory_(0) {}

    explicit Device(size_t memory_constraint)
        : tot_memory_(memory_constraint),
          avail_memory_(memory_constraint) {}

    Device(const Device &ano) = default;

    inline void AddTaskToSchedule(TaskPtr &task, LogicalTime start_time, LogicalTime exec_time) {
        task->allocated_to = node_id;
        partial_schedule_.emplace(start_time, exec_time);
        task_record_.emplace(std::piecewise_construct,
                             std::forward_as_tuple(task->node_id),
                             std::forward_as_tuple(start_time, exec_time, task->MemorySize()));
        avail_memory_ -= task->MemorySize();
    };

    inline const std::map<LogicalTime, LogicalTime> &CurrentSchedule() const {
        return partial_schedule_;
    }

    inline taskRecord GetRecordOfTask(TaskID task_id) const {
        return task_record_.at(task_id);
    }

    inline size_t AvailMemory() const {
        return avail_memory_;
    }

    inline void ResetTemp() {
        partial_schedule_.clear();
        task_record_.clear();
        avail_memory_ = tot_memory_;
    }

    virtual LogicalTime GetCompTimeOnTask(const TaskPtr &task) const = 0;
    virtual DevicePtr Clone() const = 0;

protected:
    size_t tot_memory_;
    size_t avail_memory_;
    std::map<LogicalTime, LogicalTime> partial_schedule_;   // start_time -> exec_time
    std::unordered_map<TaskID, taskRecord> task_record_;     // TaskID -> start_time + exec_time
};

class CPU : public Device {
public:
    enum SIMD {
        None,
        MMX,    // 2 int32                          |   x8
        SSE1,   // 4 float/int32,                   |   x8
        SSE2,   // 4 float/int32,  2 double/int64   |   x8
        AVX2,   // 8 float/int32,  4 double/int64   |   x8
        AVX512  // 16 float/int32, 8 double/int64   |   x8
    };

    // unit of frequency: GHz
    explicit CPU(SIMD support_isa, size_t memory_constraint, double frequency, size_t num_core)
        : Device(memory_constraint),
          ns_per_cycle_(1.0 / frequency),
          num_core_(num_core) {
        switch (support_isa) {
            case None:
                process_cap = {1, 1, 1, 1};
                break;
            case MMX:
                process_cap = {2, 1, 1, 1};
                break;
            case SSE1:
                process_cap = {4, 1, 4, 1};
                break;
            case SSE2:
                process_cap = {4, 2, 4, 2};
                break;
            case AVX2:
                process_cap = {8, 4, 8, 4};
                break;
            case AVX512:
                process_cap = {16, 8, 16, 8};
                break;
            default:
                process_cap = {1, 1, 1, 1};
                break;
        }
    }

    CPU(const CPU &ano) = default;

    LogicalTime GetCompTimeOnTask(const TaskPtr &task) const override;
    DevicePtr Clone() const override;

private:
    double ns_per_cycle_;
    size_t num_core_;

    std::vector<size_t> process_cap;    // INT32, INT64, FLOAT, DOUBLE
};

class GPU : public Device {
public:
    explicit GPU(size_t memory_constraint, double frequency, size_t num_cuda)
        : Device(memory_constraint),
          ns_per_cycle_(1.0 / frequency),
          num_cuda_(num_cuda) {}
    GPU(const GPU &ano) = default;

    LogicalTime GetCompTimeOnTask(const TaskPtr &task) const override;
    DevicePtr Clone() const override;

private:
    double ns_per_cycle_;
    size_t num_cuda_;
};

class Ascend : public Device {
public:
    explicit Ascend() = default;
    Ascend(const Ascend &ano) = default;

    LogicalTime GetCompTimeOnTask(const TaskPtr &task) const override;
    DevicePtr Clone() const override;
};

#endif //SCHEDULE_STRUCTS_DEVICE_H_
