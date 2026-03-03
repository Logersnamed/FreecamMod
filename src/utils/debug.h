#pragma once
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <mutex>

class Logger {
public:
    static void Init(const char* title = "Debug Console");
    static void Shutdown();

    static void Info(const char* fmt, ...);
    static void Warn(const char* fmt, ...);
    static void Error(const char* fmt, ...);

    static void Enable(bool enable);

private:
    static void Print(const char* level, WORD color, const char* fmt, va_list args);
    static void PrintTime();

    static inline bool initialized = false;
    static inline bool enabled = true;
    static inline std::mutex mutex;
};