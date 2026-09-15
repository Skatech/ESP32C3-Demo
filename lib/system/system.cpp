#include "system.h"

__attribute__((weak)) void SYS::led(bool flash) {
#ifdef ESP32
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(LED_BUILTIN, flash ? OUTPUT : INPUT);
#elif ESP8266
    digitalWrite(LED_BUILTIN, flash ? LOW : HIGH);
    pinMode(LED_BUILTIN, flash ? OUTPUT : INPUT);
#endif
}

__attribute__((weak)) [[noreturn]] void SYS::panic(const __FlashStringHelper* alert) {
    uint32_t counter = 0; 
    Serial.println(alert);
    while (true) {
        led(counter++ % 2);
        delay(250);
    } 
}
