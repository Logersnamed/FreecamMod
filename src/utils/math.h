#pragma once
#include <algorithm>

namespace Math {
    inline float clamp(float value) {
        return std::clamp(value, 0.0f, 1.0f);
    }

    inline float fastEase(float t) {
        t = Math::clamp(t);
        return (1.0f - t) * (1.0f - t) * (1.0f - t);
    }

    inline float quadraticEaseOut(float t) {
        t = Math::clamp(t);
        return 1.0f - (2.0f * t - 1.0f) * (2.0f * t - 1.0f);
    }
}