#ifndef SCHEDULE_ALGORITHM_GA_H_
#define SCHEDULE_ALGORITHM_GA_H_

#include "global.h"
#include "helper/helper.h"

// when all candidate is lvalue, return reference; otherwise return value
#define MAX(a, b, less) ((less)((a), (b)) ? (b) : (a))

// default parameters
constexpr static uint32_t d_max_epoch = 10000;
constexpr static clock_t d_max_duration = 2 * CLOCKS_PER_SEC;

template <typename T>
class Chromosome {
public:
    explicit Chromosome()
        : fitness(0) {}

    explicit Chromosome(std::vector<T> &&code)
            : code(std::move(code)),
              fitness(0) {}

    Chromosome(const Chromosome &ano)
        : code(ano.code),
          fitness(ano.fitness) {}

    Chromosome(Chromosome &&ano) noexcept
        : code(std::move(ano.code)),
          fitness(ano.fitness) {}

    Chromosome &operator=(const Chromosome &ano) {
        this->fitness = ano.fitness;
        this->code = ano.code;
        return *this;
    }


    static std::function<LogicalTime(Chromosome<T>&)> get_weight;

    std::vector<T> code;
    LogicalTime fitness;
};

class GA {
public:
    explicit GA(TaskGraphPtr task_graph, DeviceGraphPtr device_graph, size_t pop_size, double reproduce_rate)
        : task_graph_(std::move(task_graph)),
          device_graph_(std::move(device_graph)),
          pop_size_(pop_size),
          reproduce_rate_(reproduce_rate) {}

    ScheduleResult ScheduleWithMaxEpoch(uint32_t max_epoch = d_max_epoch);
    ScheduleResult ScheduleWithMaxDuration(clock_t max_duration = d_max_duration);

    virtual ScheduleResult GetResult() = 0;

protected:
    size_t pop_size_;
    double reproduce_rate_;

    TaskGraphPtr task_graph_;
    DeviceGraphPtr device_graph_;

    void iterOnce();

    virtual void initPopulation() = 0;
    virtual void calculateFitness() = 0;
    virtual void reproduce() = 0;
    virtual void select() = 0;
};

#endif //SCHEDULE_ALGORITHM_GA_H_
