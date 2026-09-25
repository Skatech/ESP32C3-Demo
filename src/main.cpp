#include <Arduino.h>

#define DEVICE_TITLE "ESP32-C3 Demo"
#define FIRMWARE_VERSION "1.0.56"
#define LOG_TO_SERIAL_ENABLE

#include <system.h>
#include <TaskScheduler.h>
#include <PerformanceCounter.h>
#include "init.h"
#include "http.h"

PerformanceCounter PERF;

void onConnectionStatusChangedEvent() {}

void setup() {
    initSerial();
    initFileSystem();
    loadDeviceConfig();
    initTimeSync();
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    initNetwork();
    initNameServices();
    httpInit();

    // CONNSVC.init(macroToString(F(CONN_DATA)));
    // LOG.annotateVal(F("CFG host name"), CFG[HOSTNAME]);
    // LOG.annotateVal(F("CFG conn data"), CFG[CONNDATA]);
    // LOG.annotateVal(F("CFG time data"), CFG[TIMEDATA]);
    // Serial.println(CFG[SSIDDATA]);
}

void loop() {
    PERF.start();
    SCHEDULER.poll();
    PERF.stop();
}
