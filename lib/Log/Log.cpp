#include "Log.h"

bool Log::__reserve(size_t len) {
    if (_cap < len) {
        return false;
    }
    size_t ava = _cap - _str.length();
    if (len > ava) {
        size_t rem = len - ava;
        while(rem < _str.length())
            if (_str[rem] == '\n')
                break;
            else ++rem;
        _str.remove(0, rem);
    }
    _str.reserve(len + _str.length());
    return true;
}

Log LOG(DEFAULT_LOG_CAPACITY, DEFAULT_LOG_CAPACITY);