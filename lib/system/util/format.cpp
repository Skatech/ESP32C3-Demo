#include "format.h"

#ifndef DEFAULT_FORMAT_BUFFER_SIZE
    #ifdef ESP32
    #define DEFAULT_FORMAT_BUFFER_SIZE 128
    #elif ESP8266
    #define DEFAULT_FORMAT_BUFFER_SIZE 64
    #endif
#endif

String format(const __FlashStringHelper* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[DEFAULT_FORMAT_BUFFER_SIZE];
    size_t count = vsnprintf_P(buffer, sizeof(buffer), (PGM_P)format, args);
    va_end(args);

    if (count >= sizeof(buffer)) {
        char* hbuffer = (char*)malloc(count + 1);
        if (hbuffer) {
            Serial.print(F("allocated:"));
            Serial.println(count + 1);
            va_start(args, format);
            vsnprintf_P(hbuffer, count + 1, (PGM_P)format, args);
            va_end(args);
            String str(hbuffer);
            free(hbuffer);
            return str;
        }
    }

    return buffer;
}

String rssiToString(int8_t rssi, const __FlashStringHelper* blank) {
   const __FlashStringHelper* descr = rssi < -80 ? F("very weak")
        : rssi < -70 ? F("weak")
        : rssi < -60 ? F("acceptable")
        : rssi < -50 ? F("good")
        : rssi < -40 ? F("very good") : F("excellent");
    return rssi ? format(F("%i dBm (%S)"), rssi, descr) : blank;
}

String wifiStatusToString(wl_status_t status) {
   const __FlashStringHelper* descr =
        status == WL_IDLE_STATUS ? F("Idle") :
        status == WL_NO_SSID_AVAIL ? F("No SSID") :
        status == WL_SCAN_COMPLETED ? F("Scan completed") :
        status == WL_CONNECTED ? F("Connected") :
        status == WL_CONNECT_FAILED ? F("Connect FAILED") :
        status == WL_CONNECTION_LOST ? F("Connection LOST") :
        status == WL_DISCONNECTED ? F("Disconnected") : F("No shield");
    return format(F("%hhu (%S)"), (uint8_t)status, descr);
}

String timeSpanToString(uint32_t seconds, bool full) {
    uint32_t frc = 24U * 3600UL;
    uint8_t div[] = { 1U, 24U, 60U, 60U };
    int ofs = 0;
    char buf[16];
    for(uint8_t i = 0; i < 4; ++i) {
        frc /= div[i];
        if (ofs > 0 || i > 1 || seconds >= frc || full) {
            ofs += snprintf_P(buf + ofs, sizeof(buf) - ofs, (PGM_P)(ofs < 1 ? F("%lu") : F(":%02lu")), seconds / frc);
            seconds %= frc;
        }
    }
    return buf;
}

String stringReplaceAll(String&& str, const __FlashStringHelper* sub, const __FlashStringHelper* rep) {
    while(str.lastIndexOf(sub) >= 0)
        str.replace(sub, rep);
    return str;
}

String macroToString(const __FlashStringHelper* macro) {
    return stringReplaceAll(macro, F("/#"), F("#"));
}