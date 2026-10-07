// -*- mode: c; indent-tabs-mode: nil; -*-
//
// lua_number2str for floats, which the C library here cannot print. Seven
// significant digits, as many as a float holds: plain decimals in
// [1e-4, 1e+7), d.dddddde+x outside, no trailing zeroes and no bare dot.

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

void lua_float2str(char *s, float n) {
    if (n < 0) {
        *s++ = '-';
        n = -n;
    }
    if (n == 0 || !isfinite(n)) {
        strcpy(s, n == 0 ? "0" : isnan(n) ? "nan" : "inf");
        return;
    }

    // n = m * 10^(e - 6) with m in [1e6, 1e7), in double for the seventh digit
    double m = 1e6 * n;
    int e = 0;
    while (m < 1e6) {
        m *= 10;
        e--;
    }
    while (m >= 1e7) {
        m /= 10;
        e++;
    }
    // Round half to even, as printf does
    unsigned long d = (unsigned long)m;
    double rest = m - d;
    if (rest > 0.5 || (rest == 0.5 && d % 2 == 1)) {
        d++;
    }
    if (d == 10000000) { // 9999999.5 rounds to 1e7
        d = 1000000;
        e++;
    }

    // Digits go out from the first, until none are left but zeroes
    // after the dot; point counts those still due before it.
    int scientific = e < -4 || e > 6;
    int point = scientific ? 1 : e + 1;
    if (point <= 0) {
        *s++ = '0';
        *s++ = '.';
        for (; point < 0; point++) {
            *s++ = '0';
        }
    }
    for (unsigned long p = 1000000; d != 0 || point > 0; p /= 10) {
        *s++ = '0' + d / p;
        d %= p;
        if (--point == 0 && d != 0) {
            *s++ = '.';
        }
    }
    if (scientific) {
        *s++ = 'e';
        *s++ = e < 0 ? '-' : '+';
        e = abs(e);
        if (e >= 10) {
            *s++ = '0' + e / 10;
        }
        *s++ = '0' + e % 10;
    }
    *s = '\0';
}
