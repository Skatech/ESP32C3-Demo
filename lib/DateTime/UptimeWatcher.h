#pragma once

#include <Arduino.h>
#include <limits.h>

#include <util/noncopyable.h>
#include <DateTime.h>
#include <Log.h>
#include <TaskScheduler.h>

class UptimeWatcher : Task {
    DateTime _started;

protected:

    uint16_t __task_trottle() override { return DateTime::now().isModern() ? 1U : 0U; }
    void __task_proc() {
        _started = DateTime::now() - millis() / 1000;
        SCHEDULER.remove(this);
    }

public:
    NONCOPYABLE(UptimeWatcher)
    UptimeWatcher() {
        SCHEDULER.add(this);
    }

    bool isOriginUpdated() {
        return _started.isModern();
    }

    const DateTime& getStartTime() const {
        return _started;
    }

    uint32_t getUpTime() const {
        return std::min(DateTime::now() - _started.toSecondsSinceEpoch(), (time_t)LONG_MAX);
    }
};

UptimeWatcher UPTIME;
