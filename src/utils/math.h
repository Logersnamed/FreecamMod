#pragma once
#include <algorithm>

namespace Math {
    #define PI 3.14159265358979323846f

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

    inline float toRadians(float degrees) {
        return degrees * PI / 180.0f;
	}

    inline float radToDegrees(float radians) {
        return radians * 180.0f / PI;
	}
}