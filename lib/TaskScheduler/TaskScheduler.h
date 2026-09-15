#pragma once

#include <vector>
#include <Arduino.h>
#include <util/noncopyable.h>

#ifndef DEFAULT_TASK_SCHEDULER_CAPACITY
#define DEFAULT_TASK_SCHEDULER_CAPACITY 10
#endif

class Task {
protected:
    virtual void __task_proc() = 0;
    virtual uint16_t __task_trottle() { return 1; } // zero - task disabled
    virtual ~Task() {}
    friend class TaskScheduler;
};

class TaskScheduler {
    uint16_t _index, _cycle;
    std::vector<Task*> _tasks;

public:
    NONCOPYABLE(TaskScheduler)
    TaskScheduler(uint16_t capacity = DEFAULT_TASK_SCHEDULER_CAPACITY): _index(0), _cycle(0) {
        _tasks.reserve(capacity);
    }

    uint16_t count() {
        return _tasks.size();
    }

    bool contains(Task* task) {
        for (uint16_t i = 0; i < _tasks.size(); ++i)
            if (task == _tasks[i])
                return  true;
        return false;
    }

    bool add(Task* task) {
        if (contains(task))
            return false;
        _tasks.emplace_back(task);
        return true;
    }

    bool remove(Task* task) {
        for (uint16_t i = 0; i < _tasks.size(); ++i)
            if (task == _tasks[i])
                return _tasks.end() >= _tasks.erase(i + _tasks.begin());
        return false;
    }

    bool poll();
};

extern TaskScheduler SCHEDULER;
