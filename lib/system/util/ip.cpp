#include "ip.h"

namespace IP {

IPAddress fromString(const char* str) {
    IPAddress addr; if (str) addr.fromString(str);
    return addr;
}

bool isLocal(const IPAddress& addr) {
    uint32_t mask = htonl(WiFi.subnetMask()), host = htonl(WiFi.localIP()), test = htonl(addr);
    return (host & mask) == (test & mask);
}

bool isInRange(const IPAddress& addr, const IPAddress& from, const IPAddress& last) {
    uint32_t adr = htonl(addr), min = htonl(from), max = htonl(last);
    return (max ? adr >= min && max >= adr : min ? adr == min : true);
}

uint8_t toLength(const IPAddress& addr) {
    uint32_t bits = htonl(addr);
    uint8_t mask = 0;
    while(bits >= 0xFF000000) {
        bits <<= 8;
        mask += 8;
    }
    while(bits >= 0x80000000) {
        bits <<= 1;
        mask++;
    }
    return mask;
}

String toString(const IPAddress& addr, const __FlashStringHelper* unset) {
    return addr != INADDR_NONE ? addr.toString() : (unset ? unset : F("0.0.0.0"));
}

}