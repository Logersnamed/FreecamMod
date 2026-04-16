#pragma once
#include <windows.h>
#include <array>
#include <algorithm>

#include "MinSpeedhack.h"

class Speedhack {
    double lastSpeed = 0.5;
    bool isEnabled = false;

public:
    float GetTimeScale() const { return MS::GetSpeed(); }
    void SetTimeScale(double scale) { MS::SetSpeed(scale); }

    void AddTimeScale(double delta) {
        float sensitivity = 1.0f;
        for (float i = 0.1f; i >= 0.0001f; i *= 0.1f) {
            if (MS::GetSpeed() > i) break;
            sensitivity *= 0.1f;
        }

        delta *= sensitivity;
        SetTimeScale(std::clamp(MS::GetSpeed() - delta, 0.00005, 2.0));
    }

    bool IsEnabled() const { return isEnabled; }

    void Enable() {
        isEnabled = true;
        SetTimeScale(lastSpeed);
    }

    void Disable() {
        isEnabled = false;
        lastSpeed = MS::GetSpeed();
        SetTimeScale(1.0);
    }
};