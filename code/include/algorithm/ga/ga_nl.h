#ifndef SCHEDULE_ALGORITHM_GA_NL_H_
#define SCHEDULE_ALGORITHM_GA_NL_H_

#include "ga.h"
#include "ls/ls_nl.h"

using ChromosomeNL = Chromosome<size_t>;

// default parameters
constexpr static uint32_t d_pop_size = 100;
constexpr static double d_mutate_rate = 0.01;
constexpr static double d_crossover_portion = 0.25;
constexpr static double d_mutate_portion = 0.25;
constexpr static double fitness_bias_rate = 0.01;


class GA_NL : public GA {
public:
    explicit GA_NL(TaskGraphPtr task_graph, DeviceGraphPtr device_graph,
                   uint32_t pop_size = d_pop_size, double mutate_rate = d_mutate_rate,
                   double crossover_portion = d_crossover_portion, double mutate_portion = d_mutate_portion)
            : GA(std::move(task_graph), std::move(device_graph), pop_size, crossover_portion + mutate_portion),
              mutate_rate_(mutate_rate),
              crossover_portion_(crossover_portion),
              mutate_portion_(mutate_portion),
              tot_fitness_(0) {}

    ScheduleResult GetResult() override;

protected:
    double mutate_rate_;
    double crossover_portion_;
    double mutate_portion_;
    LogicalTime tot_fitness_;
    std::vector<ChromosomeNL> population_;

    void initPopulation() override;
    void calculateFitness() override;
    void reproduce() override;
    void select() override;

    std::vector<ChromosomeNL> crossover();
    std::vector<ChromosomeNL> mutate();

    static ScheduleResult GetExecResultWithNP(const TaskGraphPtr &task_graph,
                                       const DeviceGraphPtr &device_graph,
                                       const std::vector<size_t> &priority);
};

#endif //SCHEDULE_ALGORITHM_GA_NL_H_
