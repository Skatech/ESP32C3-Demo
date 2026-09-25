#include "ConnectService.h"

#include <system.h>
#include <util/format.h>
#include <util/tokenizer.h>
#include <Log.h>

void ConnectService::init(String&& data) {
    tokenizer st(std::move(data));
    while(true) {
        const char* ssid = st.next();
        if (ssid) {
            _records.emplace_back();
            _records.back().SSID = ssid;
            _records.back().pass = st.next();
        }
        else break;
    }
    SCHEDULER.add(this);
    connect();
}

void ConnectService::connect(const String& ssid, const String& pass) {
    SYS::led(true);
    WiFi.persistent(false);
    LOG.println(format(F("Connecting to %s... "), ssid.c_str()));
    WiFi.begin(ssid, pass);
}

void ConnectService::connect(const String& ssid) {
    if (_records.size())
        connect(ssid, _records[_current].pass);
}

void ConnectService::connect(bool next) {
    if (_records.size()) {
        if (next) {
            _current++;
            _current %= _records.size();    
        }
        connect(_records[_current].SSID, _records[_current].pass);
    }
    else LOG.println("No SSID records exists");
}

String ConnectService::toString() {
    String acc;
    acc.reserve(256);
    for (uint8_t i = 0; i < _records.size(); ++i)
        acc += format(F("%s\tfails: %hhu\tRSSI: %hhi\t%S\r\n"), _records[i].SSID.c_str(),
            _records[i].fails, _records[i].rssi, _current == i ? F(" *") : F(""));
    return acc;
}

uint16_t ConnectService::__task_trottle() {
    return _statusp == WiFi.status() ? 0U : 1U;
}

void ConnectService::__task_proc() {
    const auto status = WiFi.status();
    if (_statusp != status)  {
        if (status == WL_CONNECTED) {
            LOG.annotateVal(F("Connected, RSSI"), rssiToString(WiFi.RSSI()));
            LOG.annotateVal(F("IP address"), WiFi.localIP());
            if (_records.size()) {
                _records[_current].fails = 0;
                _records[_current].rssi = WiFi.RSSI();
            }
        }
        else {
            if (status == WL_NO_SSID_AVAIL || status == WL_CONNECT_FAILED) {
                // LOG.annotateVal(F("Failed to connect"), wifiStatusToString(status));
                if (_records.size()) {
                    if (_records[_current].fails < UINT8_MAX)
                        _records[_current].fails += 1;
                    connect(true);
                }
            }
            else if (_statusp == WL_CONNECTED)
                LOG.annotateVal(F("Connection lost"), wifiStatusToString(status));
            // else LOG.annotateVal(F("Connection status changed"), wifiStatusToString(status));
        }
        SYS::led((_statusp = status) != WL_CONNECTED);
        onConnectionStatusChangedEvent();
    }
}

ConnectService CONNSVC;
