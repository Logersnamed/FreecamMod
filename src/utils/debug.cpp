#include "utils/debug.h"

void Logger::InitFile(std::filesystem::path modDirectoryPath) {
    std::filesystem::create_directories(modDirectoryPath);
    std::filesystem::path logPath = modDirectoryPath / logFileName;
    logFile.open(logPath, std::ios::out | std::ios::trunc);
}

void Logger::Print(const char* level, const char* fmt, va_list args) {
    char buffer[2048];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    if (logFile.is_open()) {
        logFile << "[" << level << "] " << buffer << std::endl;
        logFile.flush();
    }
}

void Logger::Log(const char* level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Print(level, fmt, args);
    va_end(args);
}

void Logger::Shutdown() {
    LOG_INFO("Shutting down Logger...");

    if (logFile.is_open()) logFile.close();
}