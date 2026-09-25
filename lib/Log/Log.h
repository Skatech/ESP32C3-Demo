#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include <util/noncopyable.h>

#define DEFAULT_LOG_CAPACITY 1024

class Log {
    size_t _cap;
    String _str;

    bool __reserve(size_t len);

    template <typename V> Log& __write(const V& value, uint16_t length, bool withcrlf = false) {
        if (withcrlf) {
            if (__reserve(length + 2)) {
                _str.concat(value);
                _str.concat(F("\r\n"));
            }
        }
        else if (length && __reserve(length)) {
            _str.concat(value);
        }

        #ifdef LOG_TO_SERIAL_ENABLE
        if (withcrlf)
            Serial.println(value);
        else Serial.print(value);
        #endif

        return *this;
    }

public:
    NONCOPYABLE(Log)
    Log(size_t capacity, size_t reserve = SIZE_MAX) {
        _str.reserve(min(_cap = capacity, reserve));
    }

    inline const String& toString() const {
        return _str;
    }

    Log& print(const char* value) {
        return __write(value, strlen(value));
    }

    Log& println(const char* value) {
        return __write(value, strlen(value), true);
    }

    Log& print(const __FlashStringHelper* value) {
        return __write(value, strlen_P((PGM_P)value));
    }

    Log& println(const __FlashStringHelper* value) {
        return __write(value, strlen_P((PGM_P)value), true);
    }

    Log& print(const String& value) {
        return __write(value, value.length());
    }

    Log& println(const String& value) {
        return __write(value, value.length(), true);
    }

    template<typename T> inline Log& print(const T& value) {
        return print(String(value));
    }

    template<typename T> inline Log& println(const T& value) {
        return println(String(value));
    }

    inline Log& print(const IPAddress& value) {
        return print(value != INADDR_NONE ? value.toString() : F("0.0.0.0"));
    }

    inline Log& println(const IPAddress& value) {
        return println(value != INADDR_NONE ? value.toString() : F("0.0.0.0"));
    }

    bool annotateOp(const __FlashStringHelper* title, bool success) {
        print(title); println(success ? F("... OK") : F("... FAIL"));
        return success;
    }

    template <typename P> bool annotateOp(const __FlashStringHelper* title, const P& param, bool success) {
        print(title); print(F(" (")); print(param);
        println(success ? F(")... OK") : F(")... FAIL"));
        return success;
    }

    template <typename T> void annotateVal(const __FlashStringHelper* title, const T& value) {
        print(title); print(F(": ")); println(value);
    }
};

extern Log LOG;
