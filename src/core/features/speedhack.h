#pragma once
#include <windows.h>
#include <array>
#include <algorithm>

class Speedhack {
    struct SpeedhackHook {
        void* target;
        void* detour;
        void** original;
    };

    static inline double timeScale = 1.0;
    double lastTimeScale = 0.5;
    bool isEnabled = false;

    static inline DWORD last32 = 0;
    static inline ULONGLONG last64 = 0;
    static inline LARGE_INTEGER lastQpc{};

    static inline double acc32 = 0;
    static inline double acc64 = 0;
    static inline double accQpc = 0;

    using getTickCount_t = DWORD(WINAPI*)();
    using getTickCount64_t = ULONGLONG(WINAPI*)();
    using queryPerformanceCounter_t = BOOL(WINAPI*)(LARGE_INTEGER*);

    static inline getTickCount_t origGetTickCount{};
    static inline getTickCount64_t origGetTickCount64{};
    static inline queryPerformanceCounter_t origQPC{};

    static DWORD WINAPI hkGetTickCount() {
        DWORD now = origGetTickCount();
        if (!last32) last32 = now;

        DWORD delta = now - last32;
        last32 = now;

        acc32 += delta * timeScale;
        return (DWORD)acc32;
    }

    static ULONGLONG WINAPI hkGetTickCount64() {
        ULONGLONG now = origGetTickCount64();
        if (!last64) last64 = now;

        ULONGLONG delta = now - last64;
        last64 = now;

        acc64 += delta * timeScale;
        return (ULONGLONG)acc64;
    }

    static BOOL WINAPI hkQueryPerformanceCounter(LARGE_INTEGER* lp) {
        LARGE_INTEGER now;
        BOOL r = origQPC(&now);

        if (!lastQpc.QuadPart) lastQpc = now;

        LONGLONG delta = now.QuadPart - lastQpc.QuadPart;
        lastQpc = now;

        accQpc += delta * timeScale;
        lp->QuadPart = (LONGLONG)accQpc;

        return r;
    }

public:
    float GetTimeScale() const { return timeScale; }
    void SetTimeScale(double scale) { timeScale = scale; }

	void AddTimeScale(double delta) { 
        float sensitivity = 1.0f;
        for (float i = 0.1f; i >= 0.0001f; i *= 0.1f) {
            if (timeScale > i) break;
            sensitivity *= 0.1f;
        }

        delta *= sensitivity;
        SetTimeScale(std::clamp(timeScale - delta, 0.00005, 2.0));
    }

	bool IsEnabled() const { return isEnabled; }

    void Enable() {
        isEnabled = true;
        SetTimeScale(lastTimeScale);
    }

    void Disable() {
        isEnabled = false;
        lastTimeScale = timeScale;
        SetTimeScale(1.0);
	}

    const std::array<SpeedhackHook, 3>& GetSpeedhackHooks() const {
        static std::array<SpeedhackHook, 3> hooks = {
            SpeedhackHook{ &GetTickCount, &hkGetTickCount, (void**)&origGetTickCount },
            SpeedhackHook{ &GetTickCount64, &hkGetTickCount64, (void**)&origGetTickCount64 },
            SpeedhackHook{ &QueryPerformanceCounter, &hkQueryPerformanceCounter, (void**)&origQPC },
        };
        return hooks;
    }
};