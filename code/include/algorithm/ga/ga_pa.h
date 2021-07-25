#ifndef SCHEDULE_ALGORITHM_GA_PA_H_
#define SCHEDULE_ALGORITHM_GA_PA_H_

#include "ga.h"

using ChromosomePA = Chromosome<DeviceID>;

// default parameters
constexpr static uint32_t d_pop_size = 10;
constexpr static double d_mutate_rate = 0.01;
constexpr static double d_crossover_portion = 0.25;
constexpr static double d_mutate_portion = 0.25;
constexpr static double fitness_bias_rate = 0.01;

class GA_PA : public GA {
public:
    explicit GA_PA(TaskGraphPtr task_graph, DeviceGraphPtr device_graph,
                   uint32_t pop_size = d_pop_size, double mutate_rate = d_mutate_rate,
                   double crossover_portion = d_crossover_portion, double mutate_portion = d_mutate_portion)
            : GA(std::move(task_graph), std::move(device_graph), pop_size, crossover_portion + mutate_portion),
              mutate_rate_(mutate_rate),
              crossover_portion_(crossover_portion),
              mutate_portion_(mutate_portion),
              tot_fitness_(0) {}
private:
    double mutate_rate_;
    double crossover_portion_;
    double mutate_portion_;
    LogicalTime tot_fitness_;
    std::vector<ChromosomePA> population_;

    void initPopulation() override;
    void calculateFitness() override;
    void reproduce() override;
    void select() override;
    ScheduleResult GetResult() override;

    std::vector<ChromosomePA> crossover();
    std::vector<ChromosomePA> mutate();
};

#endif //SCHEDULE_ALGORITHM_GA_PA_H_
