#ifndef SCHEDULE_INCLUDE_GLOBAL_H_
#define SCHEDULE_INCLUDE_GLOBAL_H_

#include <cstdint>
#include <ctime>
#include <functional>
#include <future>
#include <memory>
#include <random>

extern std::random_device g_random_device;

void Init();

#endif //SCHEDULE_INCLUDE_GLOBAL_H_
