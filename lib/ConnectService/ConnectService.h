#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <util/noncopyable.h>
#include <TaskScheduler.h>

void onConnectionStatusChangedEvent();

class ConnectService: public Task {
    struct Record {
        String SSID, pass;
        uint8_t fails = 0;
        int8_t rssi = 0;
    };

    std::vector<Record> _records;
    size_t _current;
    wl_status_t _statusp = WL_DISCONNECTED;

public:
    NONCOPYABLE(ConnectService);
    ConnectService() {}
    void init(String&& data);
    void connect(const String& ssid, const String& pass);
    void connect(const String& ssid);
    void connect(bool next = false);
    String toString();

protected:
    uint16_t __task_trottle() override;
    void __task_proc();
};

extern ConnectService CONNSVC;
