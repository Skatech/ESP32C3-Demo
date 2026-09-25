#pragma once

#include <Arduino.h>
#include <util/noncopyable.h>

class tokenizer {
    String _input;
    const char *_delim;
    char *_token;

public:
    NONCOPYABLE(tokenizer)
    tokenizer(String&& input, const char* delim = " "): _input(input), _delim(delim) {
        _token = const_cast<char*>(_input.c_str());
    }

    const char* next() {
        return strtok_r(_token, _delim, &_token);
    }

    const char* next(const char* replacenull) {
        const char* str = next();
        return str ? str : replacenull;
    }

    size_t read(const char** array, size_t count) {
        size_t i = 0;
        for (const char* n; i < count && (n = next()); ++i)
            array[i] = n;
        return i;
    }

    size_t read(String* array, size_t count) {
        size_t i = 0;
        for (const char* n; i < count && (n = next()); ++i)
            array[i] = n;
        return i;
    }
};