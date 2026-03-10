#include "utils/debug.h"

void Logger::InitFile(const std::string& folderPath) {
    std::filesystem::create_directories(folderPath);

    std::string filename = folderPath + "/log.txt";

    logFile.open(filename, std::ios::out | std::ios::trunc);
}

void Logger::Init(const char* title) {
    if (initialized || !enabled) return;

    AllocConsole();

    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hConsole, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode |= ENABLE_EXTENDED_FLAGS;
    SetConsoleMode(hConsole, mode);

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

    if (logFile.is_open())
        logFile.close();

    FreeConsole();
    initialized = false;
}

void Logger::Enable(bool enable) {
    enabled = enable;
    Logger::Init();
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
    std::lock_guard<std::mutex> lock(mutex);

    char buffer[2048];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    if (logFile.is_open()) {
        std::time_t t = std::time(nullptr);
        std::tm tm;
        localtime_s(&tm, &t);

        logFile << "[" << (tm.tm_hour < 10 ? "0" : "") << tm.tm_hour << ":"
            << (tm.tm_min < 10 ? "0" : "") << tm.tm_min << ":"
            << (tm.tm_sec < 10 ? "0" : "") << tm.tm_sec << "] "
            << "[" << level << "] " << buffer << std::endl;
        logFile.flush();
    }

    if (enabled && initialized) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);

        PrintTime();
        printf("[%s] %s\n", level, buffer);

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    }
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