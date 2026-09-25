#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <vector>
#include <util/format.h>
#include <util/noncopyable.h>
#include <system.h>

class ConfigFile {
    std::vector<String> _lines;
    String _fpath;

public:
    NONCOPYABLE(ConfigFile)
    ConfigFile(const __FlashStringHelper* fpath): _fpath(fpath) {}

    String& operator[] (size_t line) {
        if (line < _lines.size())
            return _lines[line];
        SYS::panic(F("Config index out of range, device HALTED"));
    }

    String copy(size_t line) {
        return (*this)[line];
    }

    size_t count() {
        return _lines.size();
    }

    String& add(String&& value) {
        _lines.emplace_back(value);
        return _lines.back();
    }

    bool load() {
        File file;
        if (LittleFS.exists(_fpath) && (file = LittleFS.open(_fpath))) {
            _lines.clear();
            while(file.available()) {
                _lines.emplace_back(file.readStringUntil('\n'));
                _lines.back().trim();
            }
            file.close();
            return true;
        }
        return false;
    }

    bool save() {
        File file = LittleFS.open(_fpath, "w", true);
        if (file) {
            for (const String& line : _lines)
                file.println(line);
            file.close();
            return true;
        }
        return false;
    }

    bool remove() {
        return LittleFS.remove(_fpath);
    }

    String toString() {
        String acc;
        for(size_t i = 0; i < _lines.size(); ++i)
            acc += format(F("#%lu\t'%s'\t%lu\r\n"), i, _lines[i].c_str(), _lines[i].length());
        return acc;
   }
};