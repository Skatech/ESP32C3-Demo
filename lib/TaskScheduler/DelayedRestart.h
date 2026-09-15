#pragma once

#include <Arduino.h>

#include <util/noncopyable.h>
#include <util/format.h>
#include <Log.h>
#include "TaskScheduler.h"

class DelayedRestart: public Task {
    uint32_t _start, _delay; 

protected:
    uint16_t __task_trottle() override { return millis() - _start >= _delay ? 1U : 0U; }
    void __task_proc() {
        SCHEDULER.remove(this);
        LOG.println(F("Device restarting..."));
        ESP.restart();
    }

public:
    bool after(uint32_t delay) {
        if (!SCHEDULER.contains(this) && SCHEDULER.add(this)) {
            _start = millis();
            _delay = delay;
            LOG.println(format(F("Device restart scheduled after %lu ms"), _delay));
            return true;
        }
        return false;
    }
};

DelayedRestart RESTART;