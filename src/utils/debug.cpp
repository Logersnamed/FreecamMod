#include "utils/debug.h"

void Logger::Init(const char* title) {
    if (initialized) return;

    AllocConsole();

    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);

    SetConsoleTitleA(title);

    initialized = true;
}

void Logger::Shutdown() {
    if (!initialized)
        return;

    FreeConsole();
    initialized = false;
}

void Logger::Enable(bool enable) {
    enabled = enable;
}

void Logger::PrintTime() {
    std::time_t t = std::time(nullptr);
    std::tm tm;
    localtime_s(&tm, &t);

    printf("[%02d:%02d:%02d] ",
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec);
}

void Logger::Print(const char* level, WORD color, const char* fmt, va_list args) {
    if (!enabled || !initialized)
        return;

    std::lock_guard<std::mutex> lock(mutex);

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);

    PrintTime();
    printf("[%s] ", level);
    vprintf(fmt, args);
    printf("\n");

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

void Logger::Info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Print("INFO", 11, fmt, args);
    va_end(args);
}

void Logger::Warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Print("WARN", 14, fmt, args);
    va_end(args);
}

void Logger::Error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Print("ERROR", 12, fmt, args);
    va_end(args);
}