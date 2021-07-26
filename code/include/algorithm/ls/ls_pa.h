#ifndef SCHEDULE_ALGORITHM_LS_PA_H_
#define SCHEDULE_ALGORITHM_LS_PA_H_

#include "ls.h"

class LS_PA : public LS {
public:
    explicit LS_PA(TaskGraphPtr task_graph, DeviceGraphPtr device_graph, TypePA processor_allocation)
        : LS(std::move(task_graph), std::move(device_graph)) {
        processor_allocation_ = std::move(processor_allocation);
    }

    void Schedule() override;

protected:
    void phaseProcessorAllocation() override;
    void phaseNodeList() override;
};

#endif //SCHEDULE_ALGORITHM_LS_PA_H_
