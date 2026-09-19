#include <Arduino.h>

#define DEVICE_TITLE "ESP32-C3 Demo"
#define FIRMWARE_VERSION "1.0.41"
#define LOG_TO_SERIAL_ENABLE

#include <system.h>
#include <TaskScheduler.h>
#include <PerformanceCounter.h>
#include <UptimeWatcher.h>
#include "init.h"
#include "http.h"
#include "WebUpdateDevice.h" ////
#include "WebUpdateFile.h"  //////
#include "WebUpdate.h"
WebUpdate UPDATE;

WebUpdateDevice WUD;
WebUpdateFile WUF;
PerformanceCounter PERF;

void onConnectionStatusChangedEvent() {
}

void setup() {
    initSerial();
    initFileSystem();
    initTimeSync();
    
    LOG.println(SNTPControl::toString());

    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    initNetwork();
    initNameServices();

    WUD.init(F("/admin/update-device"), F("/webui/pages/update-device.html")); ///////
    WUF.init(F("/admin/update-file"), F("/webui/pages/update-file.html")); ///////
    UPDATE.init(F("/admin/update"), F("/webui/pages/update.html"));

    httpInit();

    beginConnect();
}

void loop() {
    watchConnection();
    PERF.start();
    SCHEDULER.poll();
    PERF.stop();
}
