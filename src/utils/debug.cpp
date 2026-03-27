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
    SetConsoleTitleA(title);

    initialized = true;
}

void Logger::Enable(bool enable) {
    enabled = enable;
    Logger::Init();
}

void Logger::Print(const char* level, const char* fmt, va_list args) {
    std::lock_guard<std::mutex> lock(mutex);

    char buffer[2048];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    if (logFile.is_open()) {
        logFile << "[" << level << "] " << buffer << std::endl;
        logFile.flush();
    }

    printf("Freecam > [%s] %s\n", level, buffer);
}

void Logger::Log(const char* level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Print(level, fmt, args);
    va_end(args);
}

void Logger::Shutdown() {
    LOG_INFO("Shutting down Logger...");
    if (!initialized)
        return;

    if (logFile.is_open())
        logFile.close();

    FreeConsole();
    initialized = false;
}