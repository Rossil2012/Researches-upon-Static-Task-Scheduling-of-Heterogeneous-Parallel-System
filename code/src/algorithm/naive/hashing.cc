#include "naive/hashing.h"

void Hashing::Schedule() {
    ls_pa_ = new LS_PA(task_graph_, device_graph_, hashTaskToDevice());
    ls_pa_->Schedule();
}

LogicalTime Hashing::GetExecTime() {
    return ls_pa_->GetExecTime();
}

TypePA Hashing::hashTaskToDevice() {
    TypePA processor_allocation(task_graph_->GetNodeNum());

    struct deviceMock {
        DevicePtr device;
        size_t avail_memory;
    };

    std::vector<deviceMock> all_devices;
    auto device_callback = [&all_devices](DevicePtr &device) {
        all_devices.push_back({device, device->AvailMemory()});
    };
    device_graph_->Traverse(device_callback);

    auto get_avail_memory = [](deviceMock *device_mock) -> size_t {
        return device_mock->avail_memory;
    };

    auto task_callback = [&all_devices, get_avail_memory, &processor_allocation](TaskPtr &task) {
        size_t tot_avail_memory = 0;
        std::vector<deviceMock*> all_capable_device;
        for (auto &device_mock : all_devices) {
            if (device_mock.avail_memory > task->MemorySize()) {
                tot_avail_memory += device_mock.avail_memory;
                all_capable_device.push_back(&device_mock);
            }
        }

        if (all_capable_device.empty()) {
            throw "Hashing: out of memory";
        }

        auto selected_device =
                RouletteWheel<size_t, deviceMock*>(all_capable_device, tot_avail_memory, get_avail_memory);

        processor_allocation[task->node_id] = selected_device->device->node_id;
        selected_device->avail_memory -= task->MemorySize();
    };

    task_graph_->Traverse(task_callback);

    return std::move(processor_allocation);
}
