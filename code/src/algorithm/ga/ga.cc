#include "ga/ga.h"

#include <iostream>

ScheduleResult GA::ScheduleWithMaxEpoch(uint32_t max_epoch) {
    initPopulation();
    calculateFitness();

    std::cout << max_epoch << ": " << GetResult().exec_time << "\n";

    while (max_epoch-- > 0) {
        iterOnce();
        std::cout << max_epoch << ": " << GetResult().exec_time << "\n";
    }


    return GetResult();
}

ScheduleResult GA::ScheduleWithMaxDuration(clock_t max_duration) {
    auto start = clock(), now = start;

    initPopulation();
    calculateFitness();

    for (; now - start < max_duration; now = clock()) {
        iterOnce();
    }

    return GetResult();
}

void GA::iterOnce() {
    select();
    reproduce();
    calculateFitness();
}

