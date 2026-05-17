//
// Created by abcdefghkj on 17.05.2026.
//

#pragma once
#include <Windows.h>
#include <cstdio>

inline void print(const char* fmt, ...) {
    char buf[2048];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    OutputDebugStringA(buf);
}