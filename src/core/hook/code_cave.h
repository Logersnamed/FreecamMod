#pragma once
#include <vector>

#include "MinHook.h"
#include "ModUtils.h"

#include "utils/memory.h"
#include "utils/debug.h"

class CodeCave {
protected:
    uintptr_t hookAddress = 0;
    uint8_t* originalBytes = nullptr;
    size_t caveSize = 0;

public:
    virtual ~CodeCave() { Unhook(); }

    virtual void SaveOriginalBytes(uintptr_t hkAddress) = 0;
    virtual uintptr_t GetDestinationAddress() = 0;
    bool Hook(uintptr_t hkAddress);
    void Unhook();
};

class DaytimeUpdateCave : public CodeCave {
    bool* isDayTimeFrozen = nullptr;
    bool* isCycleWeatherTime = nullptr;
    int* cycleSpeed = nullptr;

public:
    bool IsCycleWeatherTime() { return isCycleWeatherTime ? *isCycleWeatherTime : false; }
    void SetCycleWeatherTime(bool enabled) { if (isCycleWeatherTime) *isCycleWeatherTime = enabled; }
    void ToggleCycleWeatherTime() { if (isCycleWeatherTime) *isCycleWeatherTime = !(*isCycleWeatherTime); }

    void SaveOriginalBytes(uintptr_t hkAddress) override;
    uintptr_t GetDestinationAddress() override;
};