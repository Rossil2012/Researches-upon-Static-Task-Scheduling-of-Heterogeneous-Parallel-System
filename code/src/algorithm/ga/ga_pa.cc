#include "ga/ga_pa.h"

template <>
std::function<LogicalTime(ChromosomePA)> ChromosomePA::get_weight =
        [](const ChromosomePA &entity) { return entity.fitness; };

// initialize randomly
void GA_PA::initPopulation() {
    TaskID task_num = task_graph_->GetNodeNum();
    DeviceID node_num = device_graph_->GetNodeNum();
    for (size_t p = 0; p < pop_size_; p++) {
        population_.emplace_back(std::vector<DeviceID>(task_num));
        for (TaskID t = 0; t < task_num; t++) {
            population_[p].code[t] = RandomWithRange(DeviceID(0), node_num);
        }
    }
}

void GA_PA::calculateFitness() {
    tot_fitness_ = 0;
    LogicalTime max_fitness = 0;
    for (auto &entity : population_) {
        entity.fitness = GetExecResultWithPA(task_graph_, device_graph_, entity.code).exec_time;
        max_fitness = std::max(max_fitness, entity.fitness);
    }

    for (auto &entity : population_) {
        entity.fitness = max_fitness - entity.fitness + LogicalTime(fitness_bias_rate * max_fitness);
        tot_fitness_ += entity.fitness;
    }
}

void GA_PA::reproduce() {
    AppendVectorWithMove(population_, crossover());
    AppendVectorWithMove(population_, mutate());
}

void GA_PA::select() {
    auto comp_func = [](const ChromosomePA &a, const ChromosomePA &b) { return a.fitness < b.fitness; };
    decltype(population_) new_population(pop_size_);

    // elitism
    ChromosomePA &elite_entity = *std::max_element(population_.begin(), population_.end(), comp_func);

    new_population[0] = elite_entity;
    PopVector(elite_entity, population_);

    // Roulette Wheel
    for (int t = 1; t < pop_size_; t++) {
        auto &winner = RouletteWheel(population_, tot_fitness_, ChromosomePA::get_weight);
        new_population[t] = winner;
        PopVector(winner, population_);
    }

//    // tournament
//    for (int t = 1; t < pop_size_; t++) {
//        uint32_t rand_low = 0, rand_high = population_.size();
//        auto idx_a = RandomWithRange(rand_low, rand_high), idx_b = RandomWithRange(rand_low, rand_high);
//        auto &winner = MAX(population_[idx_a], population_[idx_b], comp_func);
//
//        new_population[t] = winner;
//        PopVector(winner, population_);
//    }

    population_ = std::move(new_population);
}

ScheduleResult GA_PA::getResult() {
    ChromosomePA &elite_entity = *std::max_element(population_.begin(), population_.end(),
                                                           [](const ChromosomePA &a, const ChromosomePA &b) { return a.fitness < b.fitness; });
    return GetExecResultWithPA(task_graph_, device_graph_, elite_entity.code);
}

// 2-point crossover
std::vector<ChromosomePA> GA_PA::crossover() {
    decltype(population_) crs_children;

    // reproduce 2 children per iteration
    for (size_t i = 0; i < size_t(pop_size_ * crossover_portion_ * 0.5); i++) {
        // random 2 candidates
        ChromosomePA candidate[2];
        candidate[0] = RouletteWheel(population_, tot_fitness_, ChromosomePA::get_weight);
        candidate[1] = RouletteWheel(population_, tot_fitness_, ChromosomePA::get_weight);

        // random 2 points
        size_t rand_low_ = 0, rand_high_ = task_graph_->GetNodeNum();
        auto start_point = RandomWithRange(rand_low_, rand_high_), end_point = RandomWithRange(rand_low_, rand_high_);
        if (start_point > end_point) { std::swap(start_point, end_point); }

        for (DeviceID loc = start_point; loc <= end_point; loc++) {
            std::swap(candidate[0].code[loc], candidate[1].code[loc]);
            crs_children.push_back(std::move(candidate[0]));
            crs_children.push_back(std::move(candidate[1]));
        }
    }

    return std::move(crs_children);
}

// all-points mutation
std::vector<ChromosomePA> GA_PA::mutate() {
    decltype(population_) mut_children;

    // reproduce 1 child per iteration
    for (size_t i = 0; i < size_t(pop_size_ * mutate_portion_); i++) {
        // random 1 candidate
        ChromosomePA candidate = RouletteWheel(population_, tot_fitness_, ChromosomePA::get_weight);

        // random all points
        size_t rand_low = 0, rand_high = task_graph_->GetNodeNum();
        for (auto &alloc : candidate.code) {
            if (RandomWithProbability(mutate_rate_)) {
                alloc = RandomWithRange(rand_low, rand_high);
            }
        }
        mut_children.push_back(std::move(candidate));
    }

    return std::move(mut_children);
}