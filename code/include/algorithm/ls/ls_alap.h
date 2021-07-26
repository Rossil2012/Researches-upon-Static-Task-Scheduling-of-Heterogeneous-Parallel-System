#ifndef SCHEDULE_ALGORITHM_LS_ALAP_H_
#define SCHEDULE_ALGORITHM_LS_ALAP_H_

#include "ls.h"

/* List Schedule with Bottom Level as Node Priority, a.k.a. As Late As Possible */
class LS_ALAP : public LS {
public:
    explicit LS_ALAP(TaskGraphPtr task_graph, DeviceGraphPtr device_graph)
            : LS(std::move(task_graph), std::move(device_graph)) {}

protected:
    void genNodeList() override;
};

#endif //SCHEDULE_ALGORITHM_LS_ALAP_H_