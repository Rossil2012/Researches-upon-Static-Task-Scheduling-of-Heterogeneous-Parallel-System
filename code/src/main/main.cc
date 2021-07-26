#include <iostream>

#include "graph/graph.h"
#include "ga/ga_nl.h"
#include "ls/ls_asap.h"
#include "ls/ls_alap.h"

void debugPrint(const std::vector<int> &vec) {
    for (const auto &item : vec) {
        std::cout << item << " ";
    }
    std::cout << "\n";
}

void testCreateNodeListFromPriority() {
    auto graph = std::make_shared<TaskGraph>();
    std::vector<size_t> priority = {12, 13, 2, 4, 6, 8, 1, 3, 5, 7, 9, 11, 10};
    for (auto &i : priority) {
        std::cout << graph->NewNode() << " ";
    }
    std::cout << "\n";

    for (int i = 2; i < 6; i++) {
        graph->AddEdge(0, i);
        graph->AddEdge(i, 11);
    }

    for (int i = 6; i <= 10; i++) {
        graph->AddEdge(1, i);
        graph->AddEdge(i, 12);
    }

    graph->AddEdge(2, 10);

    auto nl = CreateNodeListFromPriority(graph, priority);
    for (const auto &task : nl) {
        std::cout << task->node_id << " ";
    }
    std::cout << "\n";
}

std::vector<Tasklet> GenRandomTaskFlow() {
    auto serial_num = RandomWithRange<size_t>(1, 100);

    std::vector<Tasklet> ret;
    ret.reserve(serial_num);

    for (int i = 0; i < serial_num; i++) {
        auto data_type = static_cast<DataType>(RandomWithRange<int>(INT32, DOUBLE + 1));
        ret.push_back({data_type, RandomWithRange<size_t>(1, 20000)});
    }

    return std::move(ret);
}

TaskGraphPtr makeTaskGraph() {
    const TaskID task_num = 3000;
    const double task_edge_prob = 0.01;

    auto task_graph = std::make_shared<TaskGraph>();

    for (int i = 0; i < task_num; i++) {
        task_graph->NewTask(RandomWithRange<size_t>(10, 1000), GenRandomTaskFlow());
    }

    auto task_shuffled = GenIncPriorVector<TaskID>(task_num);
    ShuffleVector(task_shuffled);

    for (auto &from_ : task_shuffled) {
        for (TaskID to_ = from_ + 1; to_ < task_num; to_++) {
            if (RandomWithProbability(task_edge_prob)) {
                task_graph->AddEdge(from_, to_);
            }
        }
    }

    return std::move(task_graph);
}

DeviceGraphPtr makeDeviceGraph() {
    const DeviceID device_num = 5;

    auto device_graph = std::make_shared<DeviceGraph>();

    device_graph->NewCPU(CPU::AVX512, size_t(4) << 30, 5.0, 10);
    device_graph->NewCPU(CPU::SSE2, size_t(500) << 20, 3.8, 4);
    device_graph->NewCPU(CPU::None, size_t(100) << 20, 2.3, 2);
    device_graph->NewGPU(size_t(4) << 30, 1.3, 2000);
    device_graph->NewGPU(size_t(4) << 30, 1.8, 4000);

    device_graph->NewNodeFinished();

    for (int i = 0 ; i < device_num; i++) {
        for (int j = i + 1; j < device_num; j++) {
            device_graph->AddEdge(i, j, RandomWithRange<size_t>(100, 10000));
        }
    }

    return std::move(device_graph);
}

void testGA() {
    auto task_graph = makeTaskGraph();
    auto device_graph = makeDeviceGraph();

    auto ga_nl = GA_NL(task_graph, device_graph, 10);

    ga_nl.ScheduleWithMaxEpoch(100);

    std::cout << ga_nl.GetResult().exec_time;
}

void testLSASAP() {
    auto task_graph = makeTaskGraph();
    auto device_graph = makeDeviceGraph();

    auto ls_asap = LS_ASAP(task_graph->Clone(), device_graph->Clone());

    ls_asap.Schedule();

    std::cout << ls_asap.GetExecTime() << "\n";

//    for (const auto &allocated_to : ls_asap.GetProcessorAllocation()) {
//        std::cout << allocated_to << " ";
//    }
//    std::cout << "\n";
//
//    for (const auto &node : ls_asap.GetNodeList()) {
//        std::cout << node->node_id << " ";
//    }
//    std::cout << "\n";
}

void testLSALAP() {
    auto task_graph = makeTaskGraph();
    auto device_graph = makeDeviceGraph();

    auto ls_alap = LS_ALAP(task_graph->Clone(), device_graph->Clone());

    ls_alap.Schedule();

    std::cout << ls_alap.GetExecTime() << "\n";
}

int main() {
//    testCreateNodeListFromPriority();
    Init();

    std::cout << "LS_ASAP:\n";
    testLSASAP();
    std::cout << "LS_ALAP:\n";
    testLSALAP();
    std::cout << "GA_NL:\n";
    testGA();

    return 0;
}