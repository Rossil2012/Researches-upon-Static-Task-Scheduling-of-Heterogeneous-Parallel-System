#ifndef SCHEDULE_ALGORITHM_LS_ASAP_H_
#define SCHEDULE_ALGORITHM_LS_ASAP_H_

#include "ls.h"

/* List Schedule with Top Level as Node Priority, a.k.a. As Soon As Possible */
class LS_ASAP : public LS {
public:
    explicit LS_ASAP(TaskGraphPtr task_graph, DeviceGraphPtr device_graph)
        : LS(std::move(task_graph), std::move(device_graph)) {}

protected:
    void genNodeList() override;
};

#endif //SCHEDULE_ALGORITHM_LS_ASAP_H_
