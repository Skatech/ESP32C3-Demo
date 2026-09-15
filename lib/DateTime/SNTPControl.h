#pragma once

#include <Arduino.h>

class SNTPControl {
public:
    static const char* getTimeZone();
    static const char* getTimeServer(uint8_t index);
    static bool isEnabled();
    static void start();
    static void stop();
    static bool setup(String&& timezone,
        String&& timeserver1, String&& timeserver2, String&& timeserver3);
    static String toString();
};
