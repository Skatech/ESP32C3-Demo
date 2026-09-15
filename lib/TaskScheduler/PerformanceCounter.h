#pragma once

#include <Arduino.h>
#include <util/noncopyable.h>
#include <util/format.h>

class PerformanceCounter {
    unsigned long _min, _max, _val;
    unsigned long long _cnt, _sum;

public:
    NONCOPYABLE(PerformanceCounter)
    PerformanceCounter() {
        reset();
    }

    void reset() {
        _cnt = _sum = 0ULL;
        _min = _max = _val = 0UL;
    }

    void start() {
        _val = max(1UL, micros());
    }

    void stop() {
        if (_val) {
            _val = micros() - _val;
            _min = min(_min, _val);
            _max = max(_max, _val);
            _sum += _val;
            _cnt++;
        }
    }

    String toString(const __FlashStringHelper* title) {
        String str; if (_cnt)
            str += format(F("%S timings, us: %lu / %lu / %lu"), title, _min, (unsigned long)(_sum / _cnt) ,_max);
        return str;
    }
};
