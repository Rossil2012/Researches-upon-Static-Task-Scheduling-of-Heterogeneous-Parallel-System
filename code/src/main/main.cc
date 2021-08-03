#include <iostream>

#include "global.h"

#include "graph/graph.h"
#include "ga/ga_nl.h"
#include "ls/ls_asap.h"
#include "ls/ls_alap.h"
#include "naive/hashing.h"
#include "naive/cpop.h"

const DeviceID device_num_ = 40;
size_t ave_memory_ = 0;

size_t task_num_ = 100;
double task_edge_prob_ = 0.01;
size_t output_size_high_ = 1000;
size_t serial_num_high_ = 100;
size_t parallel_num_high_ = 10000;

bool f_ = false;

#define RUN_ALL(epoch)          \
do {                            \
    std::cout << "Hashing:\n";  \
    testHashing(epoch);         \
    std::cout << "LS_ASAP:\n";  \
    testLSASAP(epoch);          \
    std::cout << "LS_ALAP:\n";  \
    testLSALAP(epoch);          \
    std::cout << "CPOP:\n";     \
    testCPOP(epoch);            \
    std::cout << "GA_NL:\n";    \
    testGA(epoch);              \
} while(0)

#define RUN_RANDOM_GRAPH(epoch, num, edge_prob)             \
do {                                                        \
    task_num_ = (num);                                      \
    task_edge_prob_ = (edge_prob);                          \
    RUN_ALL(epoch);                                         \
} while(0)

std::vector<Tasklet> GenRandomTaskFlow() {
    auto serial_num = RandomWithRange<size_t>(1, serial_num_high_);

    std::vector<Tasklet> ret;
    ret.reserve(serial_num);

    for (int i = 0; i < serial_num; i++) {
        auto data_type = static_cast<DataType>(RandomWithRange<int>(INT32, DOUBLE + 1));
        ret.push_back({data_type, RandomWithRange<size_t>(1, parallel_num_high_)});
    }

    return std::move(ret);
}

TaskGraphPtr makeTaskGraph() {
    static auto task_graph = std::make_shared<TaskGraph>();

    if (f_) { return task_graph; } else { task_graph = std::make_shared<TaskGraph>(); }

    task_graph->NewTask(0, std::vector<Tasklet>()); // manual source node

    for (int i = 1; i < task_num_ - 1; i++) {
        task_graph->NewTask(RandomWithRange<size_t>(1, output_size_high_), GenRandomTaskFlow());
    }

    task_graph->NewTask(0, std::vector<Tasklet>()); // manual sink node

    auto task_shuffled = GenIncPriorVector<TaskID>(task_num_);
    ShuffleVector(task_shuffled);

    for (auto &from_ : task_shuffled) {
        for (TaskID to_ = from_ + 1; to_ < task_num_; to_++) {
            if (RandomWithProbability(task_edge_prob_)) {
                task_graph->AddEdge(from_, to_);
            }
        }
    }

    for (int i = 1; i < task_num_ - 1; i++) {
        auto cur = task_graph->GetTask(i);
        if (cur->in_nodes.empty()) {
            task_graph->AddEdge(0, i);
        }
        if (cur->out_nodes.empty()) {
            task_graph->AddEdge(i, task_num_ - 1);
        }
    }

    size_t total_memory = 0;
    auto callback = [&total_memory](TaskPtr &task) {
        total_memory += task->MemorySize();
    };

    task_graph->Traverse(callback);

    ave_memory_ = total_memory / device_num_;

    return task_graph;
}

DeviceGraphPtr makeDeviceGraph() {
    static auto device_graph = std::make_shared<DeviceGraph>();

    if (f_) { return device_graph; } else { device_graph = std::make_shared<DeviceGraph>(); }

    DeviceID cur_device_num = 0;
    for (int simd = CPU::None; simd <= CPU::AVX512; simd++) {
        for (int i = 0; i < 5; i++) {
            device_graph->NewCPU(CPU::SIMD(simd), RandomWithRange<size_t>(ave_memory_, ave_memory_*3),
                RandomWithRange<int>(20, 50) / 10.0, RandomWithRange<size_t>(1, 16));
        }
        cur_device_num += 5;
    }

    for (int i = 0; i < device_num_ - cur_device_num; i++) {
        device_graph->NewGPU(RandomWithRange<size_t>(ave_memory_, ave_memory_*3),
            RandomWithRange<int>(5, 20) / 10.0, RandomWithRange<int>(500, 5000));
    }

    device_graph->NewNodeFinished();

    for (int i = 0 ; i < device_num_; i++) {
        for (int j = i + 1; j < device_num_; j++) {
            device_graph->AddEdge(i, j, RandomWithRange<size_t>(100, 10000));
        }
    }

    return device_graph;
}

void testGA(int epoch) {
    LogicalTime tot_exec_time = 0;
    int epoch_ = epoch;
    std::vector<LogicalTime> tot_exec_results;

    while (epoch_-- > 0) {
        auto task_graph = makeTaskGraph();
        auto device_graph = makeDeviceGraph();

        auto ga_nl = GA_NL(task_graph->Clone(), device_graph->Clone(), 10);

        auto exec_results = ga_nl.ScheduleWithMaxEpoch(100);

        if (tot_exec_results.empty()) {
            for (auto exec_result : exec_results) {
                tot_exec_results.push_back(exec_result);
            }
        } else {
            for (size_t i = 0; i < exec_results.size(); i++) {
                tot_exec_results[i] += exec_results[i];
            }
        }

        tot_exec_time += exec_results.back();
    }

    for (const auto &result : tot_exec_results) {
        std::cout << result / epoch << " ";
    }
    std::cout << "\n";

    std::cout << tot_exec_time / double(epoch) << "\n";
}

void testLSASAP(int epoch) {
    LogicalTime tot_exec_time = 0;
    int epoch_ = epoch;
    while (epoch_-- > 0) {
        auto task_graph = makeTaskGraph();
        auto device_graph = makeDeviceGraph();

        auto ls_asap = LS_ASAP(task_graph->Clone(), device_graph->Clone());

        ls_asap.Schedule();
        tot_exec_time += ls_asap.GetExecTime();
    }
    std::cout << tot_exec_time / double(epoch) << "\n";
}

void testLSALAP(int epoch) {
    LogicalTime tot_exec_time = 0;
    int epoch_ = epoch;

    while (epoch_-- > 0) {
        auto task_graph = makeTaskGraph();
        auto device_graph = makeDeviceGraph();

        auto ls_alap = LS_ALAP(task_graph->Clone(), device_graph->Clone());

        ls_alap.Schedule();
        tot_exec_time += ls_alap.GetExecTime();
    }

    std::cout << tot_exec_time / double(epoch) << "\n";
}

void testHashing(int epoch) {
    LogicalTime tot_exec_time = 0;
    int epoch_ = epoch;

    while (epoch_-- > 0) {
        auto task_graph = makeTaskGraph();
        auto device_graph = makeDeviceGraph();

        auto hashing = Hashing(task_graph->Clone(), device_graph->Clone());

        hashing.Schedule();
        tot_exec_time += hashing.GetExecTime();
    }

    std::cout << tot_exec_time / double(epoch) << "\n";
}

void testCPOP(int epoch) {
    LogicalTime tot_exec_time = 0;
    int epoch_ = epoch;

    while (epoch_-- > 0) {
        auto task_graph = makeTaskGraph();
        auto device_graph = makeDeviceGraph();

        auto cpop = CPOP(task_graph->Clone(), device_graph->Clone());
        cpop.Schedule();
        tot_exec_time += cpop.GetExecTime();
    }

    std::cout << tot_exec_time / double(epoch) << "\n";
}

int main() {
    try {
        Init();
        RUN_RANDOM_GRAPH(50, 100, 0.01);
//    RUN_RANDOM_GRAPH(50, 1000, 0.01);
//    RUN_RANDOM_GRAPH(50, 10000, 0.01);
    } catch (const char *msg) {
        std::cout << msg << "\n";
        return -1;
    }

    return 0;
}