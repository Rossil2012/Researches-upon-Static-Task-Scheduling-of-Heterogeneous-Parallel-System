#include "ga/ga.h"

#include <iostream>

ScheduleResult GA::ScheduleWithMaxEpoch(uint32_t max_epoch) {
    initPopulation();
    calculateFitness();

    while (max_epoch-- > 0) {
        iterOnce();
        std::cout << max_epoch << ": " << getResult().exec_time << "\n";
    }


    return getResult();
}

ScheduleResult GA::ScheduleWithMaxDuration(clock_t max_duration) {
    auto start = clock(), now = start;

    initPopulation();
    calculateFitness();

    for (; now - start < max_duration; now = clock()) {
        iterOnce();
    }

    return getResult();
}

void GA::iterOnce() {
    select();
    reproduce();
    calculateFitness();
}

