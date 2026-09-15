#include "TaskScheduler.h"

bool TaskScheduler::poll() {
    for(uint8_t n = 0; n < _tasks.size(); ++n) {
        if (_index >= _tasks.size()) {
            _index = 0;
            _cycle++;
        }
        Task& task = *_tasks[_index++];
        auto trottle = task.__task_trottle();
        if (trottle == 1U || (trottle && ((_cycle + _index) % trottle < 1U))) {
            task.__task_proc();
            return true;
        }
    }
    return false;
}

TaskScheduler SCHEDULER;
