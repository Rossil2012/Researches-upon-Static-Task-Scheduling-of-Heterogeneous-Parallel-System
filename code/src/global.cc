#include "global.h"

#include <ctime>

std::random_device g_random_device;

void Init() {
    srand(unsigned(time(nullptr)));
    auto rand_times = rand() % 100;
    for (int i = 0; i < rand_times; i++) {
        g_random_device();
    }
}