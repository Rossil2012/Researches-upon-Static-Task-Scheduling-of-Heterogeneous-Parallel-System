#include "ga/ga_nl.h"

#include <unordered_set>

template <>
std::function<LogicalTime(ChromosomeNL&)> ChromosomeNL::get_weight =
        [](const ChromosomeNL &entity) { return entity.fitness; };

void GA_NL::initPopulation() {
    for (int i = 0; i < pop_size_; i++) {
        population_.emplace_back(GenIncPriorVector<size_t>(task_graph_->GetNodeNum()));
        ShuffleVector(population_[i].code);
    }
}

void GA_NL::calculateFitness() {
    tot_fitness_ = 0;
    LogicalTime max_fitness = 0;
    std::vector<std::future<ScheduleResult>> async_res; async_res.reserve(pop_size_);
    for (auto &entity : population_) {
        async_res.push_back(std::async(std::launch::async,
                                       GA_NL::GetExecResultWithNP, task_graph_, device_graph_, entity.code));
    }

    for (size_t i = 0; i < population_.size(); i++) {
        auto &entity = population_[i];
        entity.fitness = async_res[i].get().exec_time;
        max_fitness = std::max(max_fitness, entity.fitness);
    }

    for (auto &entity : population_) {
        entity.fitness = max_fitness - entity.fitness + LogicalTime(fitness_bias_rate * max_fitness);
        tot_fitness_ += entity.fitness;
    }
}

void GA_NL::reproduce() {
    auto crs_children = crossover(), mut_children = mutate();
    AppendVectorWithMove(population_, std::move(crs_children));
    AppendVectorWithMove(population_, std::move(mut_children));
}

void GA_NL::select() {
    auto comp_func = [](const ChromosomeNL &a, const ChromosomeNL &b) { return a.fitness < b.fitness; };
    decltype(population_) new_population(pop_size_);
    decltype(tot_fitness_) new_tot_fitness_ = 0;

    // elitism
    ChromosomeNL &elite_entity = *std::max_element(population_.begin(), population_.end(), comp_func);

    tot_fitness_ -= elite_entity.fitness; new_tot_fitness_ += elite_entity.fitness;
    new_population[0] = elite_entity;
    PopVector(elite_entity, population_);


    // Roulette Wheel
    for (int t = 1; t < pop_size_; t++) {
        auto &winner = RouletteWheel(population_, tot_fitness_, ChromosomeNL::get_weight);
        tot_fitness_ -= winner.fitness; new_tot_fitness_ += winner.fitness;
        new_population[t] = winner;
        PopVector(winner, population_);
    }

    population_ = std::move(new_population);
    tot_fitness_ = new_tot_fitness_;
}

ScheduleResult GA_NL::GetResult() {
    ChromosomeNL &elite_entity = *std::max_element(population_.begin(), population_.end(),
        [](const ChromosomeNL &a, const ChromosomeNL &b) { return a.fitness < b.fitness; });
    return GetExecResultWithNP(task_graph_, device_graph_, elite_entity.code);
}

// subtour exchange crossover
std::vector<ChromosomeNL> GA_NL::crossover() {
    decltype(population_) crs_children;

    // complexity: O(NlogM)
    // reproduce 2 children per iteration
    for (size_t i = 0; i < size_t(pop_size_ * crossover_portion_ * 0.5); i++) {
        // random 2 candidates
        ChromosomeNL candidate[2];
        candidate[0] = RouletteWheel(population_, tot_fitness_, ChromosomeNL::get_weight);
        candidate[1] = RouletteWheel(population_, tot_fitness_, ChromosomeNL::get_weight);

        // random 2 points
        size_t rand_low_ = 0, rand_high_ = task_graph_->GetNodeNum();
        auto start_point = RandomWithRange(rand_low_, rand_high_), end_point = RandomWithRange(rand_low_, rand_high_);
        if (start_point > end_point) { std::swap(start_point, end_point); }

        auto &to_cut = candidate[0];        // to_cut is the candidate with continuous region cut for swap,
        auto &to_compensate = candidate[1]; // to_compensate's swapped genes are ones within to_cut's swap region
        std::unordered_set<size_t> to_swap(to_cut.code.begin() + start_point, to_cut.code.begin() + end_point + 1);

        size_t swap_cnt = 0;
        for (auto &gene : to_compensate.code) {
            if (to_swap.find(gene) != to_swap.end()) {
                std::swap(to_cut.code[start_point + swap_cnt++], gene);
            }
        }
        crs_children.push_back(to_cut);
        crs_children.push_back(to_compensate);
    }

    return std::move(crs_children);
}

// swap mutation
std::vector<ChromosomeNL> GA_NL::mutate() {
    decltype(population_) mut_children;

    // reproduce 1 child per iteration
    for (size_t i = 0; i < size_t(pop_size_ * mutate_portion_); i++) {
        // random 1 candidate
        ChromosomeNL candidate = RouletteWheel(population_, tot_fitness_, ChromosomeNL::get_weight);

        // random 2 points to swap
        size_t rand_low = 0, rand_high = task_graph_->GetNodeNum();
        for (size_t j = 0; j < size_t(candidate.code.size() * mutate_rate_ + 1); j++) {
            auto point_1 = RandomWithRange(rand_low, rand_high), point_2 = RandomWithRange(rand_low, rand_high);
            std::swap(candidate.code[point_1], candidate.code[point_2]);
        }

        mut_children.push_back(std::move(candidate));
    }

    return std::move(mut_children);
}

ScheduleResult GA_NL::GetExecResultWithNP(const TaskGraphPtr &task_graph,
                                   const DeviceGraphPtr &device_graph,
                                   const std::vector<size_t> &priority) {

    auto task_graph_clone = task_graph->Clone();
    auto nl = CreateNodeListFromPriority(task_graph_clone, priority);
    LS_NL ls_nl(std::move(task_graph_clone), device_graph->Clone(), std::move(nl));
    ls_nl.Schedule();
    return {
            .allocation = ls_nl.GetProcessorAllocation(),
            .exec_time = ls_nl.GetExecTime(),
    };
}
