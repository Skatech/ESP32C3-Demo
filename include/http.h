#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <uri/UriRegex.h>

#include <util/format.h>
#include <Log.h>
#include <DateTime.h>
#include <UptimeWatcher.h>
#include <PerformanceCounter.h>
#include <DelayedRestart.h>
#include <WebServerEx.h>
#include <WebPageBuilder.h>
#include <WebUpdate.h>
#include <ConnectService.h>

#define HTTP_SERVER_PORT 80

WebServerEx HTTP(HTTP_SERVER_PORT);
WebUpdate UPDATE;
extern PerformanceCounter PERF;

void httpInit() {
    HTTP.on(F("/"), []() {
        HTTP.html(format(F("<h1>Hello from ESP32-C3!!!</h1><h2>System time: %s</h2><h2>Authenticated: %S</h2>"),
            DateTime::now().toString().c_str(), HTTP.isAuthenticated() ? F("Yep") : F("Nop")));
    });

    HTTP.on(F("/admin/cfg/save"), []() {
        if (HTTP.isAuthenticated())
            HTTP.text(saveDeviceConfig()
                ? F("Device config saving... OK") : F("Device config saving... FAILED"));
        else HTTP.forbidden();
    });

    HTTP.on(F("/admin/cfg"), []() {
        if (HTTP.isAuthenticated()) {
            String a = HTTP.arg(F("id"));
            if (a.length()) {
                String val = HTTP.arg(F("val"));
                int idx = a.toInt();
                if (val.length()) {
                    if (idx < CFG.count())
                        CFG[idx] = val;
                    else CFG.add(std::move(val));
                }
                HTTP.text(idx == DeviceConfig::SSIDDATA
                    ? F("<<SSIDDATA>>") : idx < CFG.count() ? CFG[(DeviceConfig)idx] : F("<<BAD-IDX>>"));
            }
            else HTTP.text("Invalid request format");
        }
        else HTTP.forbidden();
    });

    // HTTP.on(F("/test/cfg/del"), []() {
    //     HTTP.text(LittleFS.remove(F("/test.txt")) ? F("Config loading... OK") : F("Config loading... FAIL"));
    // });

    // HTTP.on(F("/test/cfg/load"), []() {
    //     HTTP.text(cfg.load(F("/test.txt")) ? F("Config loading... OK") : F("Config loading... FAIL"));
    // });

    // HTTP.on(F("/login"), []() {
    //     if (HTTP.isAuthenticated())
    //         HTTP.redirect();
    //     else HTTP.requestAuthentication();
    // });

    HTTP.on(F("/test/wifi-next"), []() {
        HTTP.text(F("Testing WiFi next(true)"));
        HTTP.client().stop();
        CONNSVC.connect(true);
    });

    HTTP.on(F("/data/wifi"), HTTP_GET, []() {
        if (HTTP.isAuthenticated())
            HTTP.text(CONNSVC.toString());
        else HTTP.forbidden();
    });

    HTTP.on(UriRegex(F("/admin/ssid/([a-zA-Z0-9]+)")), HTTP_GET, []() {
        if (HTTP.isAuthenticated()) {
            String ssid = HTTP.pathArg(0);
            HTTP.text(String(F("Connecting to SSID: ")) + ssid);
            HTTP.client().stop();
            CONNSVC.connect(ssid);
        }
        else HTTP.forbidden();
    });

    HTTP.on(F("/admin"), []() {
        if (HTTP.isAuthenticated())
            HTTP.file(F("/webui/pages/admin.html"));
        else HTTP.requestAuthentication();
    });

    HTTP.on(F("/admin/logs"), HTTP_GET, []() {
        if (HTTP.isAuthenticated())
            HTTP.file(F("/webui/pages/admin-logs.html"));
        else HTTP.forbidden();
    });

    HTTP.on(F("/admin/logout"), HTTP_GET, []() {
        if (HTTP.isAuthenticated())
            HTTP.requestAuthentication();
        else HTTP.redirect();
    });

    HTTP.on(F("/admin/restart"), HTTP_GET, []() {
        if (HTTP.isAuthenticated()) {
            HTTP.client().setNoDelay(true);
            HTTP.html(WebPageBuilder(F("Device RESTART"))
                .addReloadScript(5000, F("/admin")).toString(
                    F("<h2>SUCCEEDED</h2><h4>Wait until device restarted...</h4>")));
            SYS::restartAfter(500);
        }
        else HTTP.forbidden();
    });

    HTTP.on(F("/data/status"), HTTP_GET, []() {
        if (HTTP.isAuthenticated()) {
            String report(F(DEVICE_TITLE));
            report += format(F("\nFirmware version: %s"), String(F(FIRMWARE_VERSION)).c_str());
            report += format(F("\nConnected to: %s"), WiFi.SSID().c_str());
            report += format(F("\nSignal Strength (RSSI): %s"), rssiToString(WiFi.RSSI()).c_str());
            report += format(F("\nHostname: %s"), WiFi.getHostname());

            report += format(F("\nMAC address: %s"), WiFi.macAddress().c_str());
            report += format(F("\nIP address: %s / %hhu"), WiFi.localIP().toString().c_str(), IP::toLength(WiFi.subnetMask()));
            report += format(F("\nGateway: %s"), WiFi.gatewayIP().toString().c_str());
            report += format(F("\nDNS: %s, %s"), WiFi.dnsIP(0).toString().c_str(), IP::toString(WiFi.dnsIP(1)).c_str());
            
            report += format(F("\n%s"), SNTPControl::toString().c_str());
            report += format(F("\nTime: %s"), DateTime::now().toString().c_str());
            report += format(F("\nStarted: %s"), UPTIME.getStartTime().toString().c_str());
            report += format(F("\nUptime: %s"), timeSpanToString(UPTIME.getUpTime()).c_str());
            report += format(F("\nHeap free: %lu"), ESP.getFreeHeap());
            report += format(F("\nDisk used / total: %lu / %lu"), LittleFS.usedBytes(), LittleFS.totalBytes());

            // for(uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; ++i)
            //     if (SOCK.server().clientIsConnected(i))
            //         report += format(F("\nWS-connection #%hhu: %s"),
            //             i, SOCK.server().remoteIP(i).toString().c_str());
            
            report += format(F("\nScheduler tasks / active: %hu / %hu"), SCHEDULER.count(), SCHEDULER.count(true));
            report += PERF.toString(F("\nScheduler"));
            HTTP.text(report);
        }
        else HTTP.forbidden();
    });

    HTTP.on(F("/data/log"), []() {
        if(HTTP.isAuthenticated())
            HTTP.text(LOG.toString());
        else HTTP.forbidden();
    });

    HTTP.onNotFound([]() {
        HTTP.html(WebPageBuilder(F("Device ERROR")).addIndexLink().addAdminLink()
            .toString(format(F("<h2>404 - NOT FOUND:<h2><h4>%s</h4>"), HTTP.uri())));
        // HTTP.html(format(F("<h1>404 - Not found</h1><h3>url: %s</h3>"),
        //     HTTP.uri().c_str()), 404);
    });

    UPDATE.init();
    HTTP.serveStatic("/", LittleFS, "/webui/static/", "no-cache"); //"max-age=3600" or "no-cache"    
    HTTP.begin();
    // HTTP.setAuthData(F(AUTH_DATA));
    HTTP.setAuthData(CFG.copy(AUTHDATA));
    MDNS.addService(F("http"), F("tcp"), HTTP_SERVER_PORT);
    SCHEDULER.add(&HTTP);
}
