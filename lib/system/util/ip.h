#pragma once

#include <Arduino.h>
#include <WiFi.h>

namespace IP {
    IPAddress fromString(const char* str);
    bool isLocal(const IPAddress& addr);
    bool isInRange(const IPAddress& addr, const IPAddress& from, const IPAddress& last);
    uint8_t toLength(const IPAddress& addr);
    String toString(const IPAddress& addr, const __FlashStringHelper* unset = nullptr);
}

