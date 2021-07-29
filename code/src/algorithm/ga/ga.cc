#include "ga/ga.h"

#include <iostream>

std::vector<LogicalTime> GA::ScheduleWithMaxEpoch(uint32_t max_epoch) {
    initPopulation();
    calculateFitness();

    std::vector<LogicalTime> exec_results;
    exec_results.push_back(GetResult().exec_time);

    while (max_epoch-- > 0) {
        iterOnce();
        exec_results.push_back(GetResult().exec_time);
    }

    return std::move(exec_results);
}

std::vector<LogicalTime> GA::ScheduleWithMaxDuration(clock_t max_duration) {
    auto start = clock(), now = start;

    initPopulation();
    calculateFitness();

    std::vector<LogicalTime> exec_results;
    exec_results.push_back(GetResult().exec_time);

    for (; now - start < max_duration; now = clock()) {
        iterOnce();
        exec_results.push_back(GetResult().exec_time);
    }

    return std::move(exec_results);
}

void GA::iterOnce() {
    select();
    reproduce();
    calculateFitness();
}

