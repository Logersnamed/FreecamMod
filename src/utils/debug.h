#pragma once
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <string>
#include <fstream>
#include <filesystem>
#include <intrin.h>

#ifdef LOG_LOCATION
    #define __FILENAME__ Logger::Filename(__FILE__)
    #define LOG_INFO(fmt, ...)  Logger::Log("INFO", "[%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
    #define LOG_WARN(fmt, ...)  Logger::Log("WARN", "[%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
    #define LOG_ERROR(fmt, ...) Logger::Log("ERROR", "[%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
#else
    #define LOG_INFO(fmt, ...)  Logger::Log("INFO", fmt, ##__VA_ARGS__)
    #define LOG_WARN(fmt, ...)  Logger::Log("WARN", fmt, ##__VA_ARGS__)
    #define LOG_ERROR(fmt, ...) Logger::Log("ERROR", fmt, ##__VA_ARGS__)
#endif

class Logger {
	static inline const std::string logFileName = "log.txt";

public:
    static void Shutdown();

    static void InitFile(std::filesystem::path folderPath);

    static void Log(const char* level, const char* fmt, ...);

    static inline const char* Filename(const char* path) {
        const char* p1 = strrchr(path, '\\');
        const char* p2 = strrchr(path, '/');
        const char* p = p1 > p2 ? p1 : p2;
        return p ? p + 1 : path;
    }

private:
    static void Print(const char* level, const char* fmt, va_list args);

    static inline std::ofstream logFile;
};

struct ScopeProfiler {
    const char* func_name;
    unsigned __int64 start;

    ScopeProfiler(const char* func_name) : func_name(func_name) {
        _mm_lfence();
        start = __rdtsc();
    }

    ~ScopeProfiler() {
        _mm_lfence();
        LOG_INFO("%s: %llu cycles", func_name, __rdtsc() - start);
    }
};

#define PROFILE_SCOPE() ScopeProfiler pm_(__func__)