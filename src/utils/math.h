#pragma once
#include <algorithm>

#include "utils/types.h"

namespace Math {
    constexpr float PI = 3.14159265358979323846f;

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

    template<typename T>
    inline T lerp(const T& a, const T& b, float t) {
        return a + t * (b - a);
    }

    inline float3 lerp(const float3& a, const float3& b, float t) {
        return float3(
            lerp(a.x, b.x, t),
            lerp(a.y, b.y, t),
            lerp(a.z, b.z, t)
        );
    }

    inline float smoothstep(float a, float b, float x) {
        const float t = clamp((x - a) / (b - a));
        return t * t * (3.0f - 2.0f * t);
    }

    inline float smoothstep(float x) {
        return smoothstep(0.0f, 1.0f, x);
    }
}