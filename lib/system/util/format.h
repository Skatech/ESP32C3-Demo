#pragma once

#include <Arduino.h>
#include <WiFi.h>

/* Returns formatted string using vsnprintf_P, default buffer size 64
%.Nf    fixed point, pad with zero if integer, max characters if string
%0Ni    padding with zeroes
d,i     signed decimal integer
u o xX  unsigned decimal, unsigned octal, unsigned hexadecimal integer
fF eE gG aA decimal float/double, scentific, shortest, hexadecimal floating
c s S   character, string, flash string

p n     void*, int*
hh h l ll - char, short, long, long long
j z t   intmax_t size_t ptrdiff_t
L       long double

-       justify left, by default right
+       force print sign
SPACE   print space when no sign
(#)       precede hex with 0x, include decimal point if float
0	    pads the number with zeroes instead of spaces (%0Ni)
.N N    precision, width (%.Nf) (%Nf)
.* *    precision, width from additional argument
https://cplusplus.com/reference/cstdio/printf  may require build_flags: -Wl,-u,vfprintf -lprintf_flt */
String format(const __FlashStringHelper* format, ...);

// Return string: "-NN dBm (quality description)"
String rssiToString(int8_t rssi, const __FlashStringHelper* blank = F("-"));

String wifiStatusToString(wl_status_t status);

// Return time-span string: "D:HH:MM:SS"
String timeSpanToString(uint32_t seconds, bool full = false);

String stringReplaceAll(String&& str, const __FlashStringHelper* sub, const __FlashStringHelper* rep);

String macroToString(const __FlashStringHelper* macro);