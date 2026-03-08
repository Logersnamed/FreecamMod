#pragma once
#include <algorithm>

inline float fastEase(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    float ease = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
    return 1.0f - ease;
}

inline float quadraticEaseOut(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return 1.0f - (2.0f * t - 1.0f) * (2.0f * t - 1.0f);
}