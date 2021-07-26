#ifndef SCHEDULE_ALGORITHM_LS_NL_H_
#define SCHEDULE_ALGORITHM_LS_NL_H_

#include "ls.h"

class LS_NL : public LS {
public:
    explicit LS_NL(TaskGraphPtr task_graph, DeviceGraphPtr device_graph, TypeNL node_list)
        : LS(std::move(task_graph), std::move(device_graph)) {
        node_list_ = std::move(node_list);
    }
};

#endif //SCHEDULE_ALGORITHM_LS_NL_H_
