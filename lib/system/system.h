#pragma once

#include <Arduino.h>

namespace SYS {
    void led(bool flash);
    [[noreturn]] void panic(const __FlashStringHelper* alert);
}
