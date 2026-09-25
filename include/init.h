#pragma once

#include <Arduino.h>
#include <ESPmDNS.h>
#include <LittleFS.h>

#include <system.h>
#include <util/format.h>
#include <util/tokenizer.h>
#include <util/ip.h>
#include <Log.h>
#include <SNTPControl.h>
#include <ConnectService.h>
#include "ConfigFile.h"

enum DeviceConfig { HOSTNAME, CONNDATA, TIMEDATA, AUTHDATA, SSIDDATA };
ConfigFile CFG(F("/config/device.cfg"));

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

void loadDeviceConfig() {
    if (!LOG.annotateOp(F("Loading device configuration"), CFG.load()))
        SYS::panic(F("Unable to load device configuration, device HALTED"));
}

bool saveDeviceConfig() {
    return LOG.annotateOp(F("Saving device configuration"), CFG.save());
}

void initTimeSync() {
    tokenizer st(CFG.copy(TIMEDATA));
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
    tokenizer st(CFG.copy(CONNDATA));
    IPAddress addr = IP::fromString(st.next()),
        mask = IP::fromString(st.next()), gate = IP::fromString(st.next()),
        dns1 = IP::fromString(st.next()), dns2 = IP::fromString(st.next());

    // if (!LOG.annotateOp(F("Loading IP configuration"), addr.fromString(st.next())
    //         && mask.fromString(st.next()) && gate.fromString(st.next()))) {
    //     SYS::panic(F("Unable to load IP configuration, device HALTED"));
    // }
    // dns1.fromString(st.next());
    // dns2.fromString(st.next());
    
    WiFi.disconnect();
    if (!LOG.annotateOp(F("Changing IP configuration"), WiFi.mode(WIFI_STA)
            && WiFi.config(addr, gate, mask, dns1, dns2) && WiFi.hostname(CFG[HOSTNAME]))) {
        SYS::panic(F("Unable to configure IP, device HALTED"));
    }
    CONNSVC.init(String(CFG[SSIDDATA])); 
}

bool initNameServices() {
    String host = CFG.copy(HOSTNAME);
    if (!host.isEmpty()) {
        return LOG.annotateOp(F("Initializing mDNS"), host + F(".local"),
            MDNS.begin(host.c_str())/* && MDNS.addService(F("http"), F("tcp"), 80)*/);
    }
    return false;
}
