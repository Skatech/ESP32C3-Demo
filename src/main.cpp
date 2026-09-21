#include <Arduino.h>

#define DEVICE_TITLE "ESP32-C3 Demo"
#define FIRMWARE_VERSION "1.0.42"
#define LOG_TO_SERIAL_ENABLE

#include <system.h>
#include <TaskScheduler.h>
#include <PerformanceCounter.h>
#include <UptimeWatcher.h>
#include "init.h"
#include "http.h"

PerformanceCounter PERF;

void onConnectionStatusChangedEvent() {
}

void setup() {
    initSerial();
    initFileSystem();
    initTimeSync();
    
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    initNetwork();
    initNameServices();
    
    httpInit();

    beginConnect();
}

void loop() {
    watchConnection();
    PERF.start();
    SCHEDULER.poll();
    PERF.stop();
}
