#pragma once

#include <Arduino.h>
#include <ESPmDNS.h>
#include <LittleFS.h>

#include <system.h>
#include <util/format.h>
#include <util/tokenizer.h>
#include <Log.h>
#include <SNTPControl.h>

void initSerial() {
    Serial.setDebugOutput(false);
    Serial.begin(SERIAL_SPEED);
    delay(1000);
    Serial.println(F("\n\n>>>>>>>>>>"));
    LOG.println(format(F("%S v%S"), F(DEVICE_TITLE), F(FIRMWARE_VERSION)));
}

void initFileSystem() {
    if(!LOG.annotateOp(F("Initializing filesystem"), LittleFS.begin()))
        SYS::panic(F("Unable to initialize filesystem, device HALTED"));
}

void initTimeSync() {
    tokenizer st(F(SYNC_TIME));
    String tz = st.next(), s1 = st.next(), s2 = st.next(), s3 = st.next();
    if (tz.length() && s1.length()) {
        if (LOG.annotateOp(F("Initializing SNTP"), tz,
                SNTPControl::setup(std::move(tz), std::move(s1), std::move(s2), std::move(s3)))) {
            LOG.println(SNTPControl::toString());
        }
        else SYS::panic(F("Unable to initialize SNTP, device HALTED"));
    }
    else LOG.println(F("SNTP settings not defined"));
}

void initNetwork() {
    String host(F(CONN_HOST));
    IPAddress addr, gate, mask, dns1, dns2;
    if (!LOG.annotateOp(F("Loading IP configuration"),
            addr.fromString(F(CONN_ADDR)) && gate.fromString(F(CONN_GATE)) && mask.fromString(F(CONN_MASK)))) {
        SYS::panic(F("Unable to load IP configuration, device HALTED"));
    }
    dns1.fromString(F(CONN_DNS1));
    dns2.fromString(F(CONN_DNS2));

    WiFi.disconnect();
    if (!LOG.annotateOp(F("Changing IP configuration"), WiFi.mode(WIFI_STA)
            && WiFi.config(addr, gate, mask, dns1, dns2) && WiFi.hostname(host))) {
        SYS::panic(F("Unable to configure IP, device HALTED"));
    }
}

void beginConnect(const String& customSSID = "") {
    SYS::led(true);
    // MDNS.end();
    WiFi.persistent(false);
    String ssid(F(CONN_SSID)), pass(F(CONN_PASS));
    ssid = customSSID.isEmpty() ? ssid : customSSID;
    if (!LOG.annotateOp(F("Starting connection"), ssid, WiFi.disconnect() 
            && (WiFi.begin(ssid, pass) == WL_DISCONNECTED || WiFi.status() == WL_CONNECTED))) {
        SYS::panic(F("Unable to start connection, device HALTED"));
    }
}

bool initNameServices() {
    String host(F(CONN_HOST));
    if (!host.isEmpty()) {
        return LOG.annotateOp(F("Initializing mDNS"), String(WiFi.getHostname()) + F(".local"),
            MDNS.begin(WiFi.getHostname())/* && MDNS.addService(F("http"), F("tcp"), 80)*/);
    }
    return false;
}

void onConnectionStatusChangedEvent();

bool watchConnection() {
    static wl_status_t statusp = WL_DISCONNECTED;
    const auto status = WiFi.status();
    if (status != statusp)  {
        if (status == WL_CONNECTED) {
            LOG.println(format(F("Connected %s, RSSI: %s\r\nIP address: %s"), WiFi.SSID().c_str(),
                rssiToString(WiFi.RSSI()).c_str(), WiFi.localIP().toString().c_str()));

            // LOG.println(format(F("Connected to AP %s:\r\n  - RSSI: %s\r\n  - IP: %s"),
            //     WiFi.SSID().c_str(), rssiToString(WiFi.RSSI()).c_str(), WiFi.localIP().toString().c_str()));
            SYS::led(false);
            // initNameServices();
        }
        else if (statusp == WL_CONNECTED) {
            LOG.println(F("Connection LOST"));
            SYS::led(true);
        }
        else {
            LOG.annotateVal(F("Connection status changed"), wifiStatusToString(status));
            SYS::led(status != WL_CONNECTED);
        }
        onConnectionStatusChangedEvent();
    }
    // if(status == WL_CONNECTED && MDNS.isRunning())
    //     MDNS.update();
    return (statusp = status) == WL_CONNECTED;
}