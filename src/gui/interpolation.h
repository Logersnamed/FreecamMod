#include "utils/types.h"

static float CatmullRom(float A, float B, float C, float D, float t) {
    float a = -A / 2.0f + (3.0f * B) / 2.0f - (3.0f * C) / 2.0f + D / 2.0f;
    float b = A - (5.0f * B) / 2.0f + 2.0f * C - D / 2.0f;
    float c = -A / 2.0f + C / 2.0f;
    float d = B;

    return a * t * t * t + b * t * t + c * t + d;
}

template<typename T>
struct CatmullRomInterpolation;

template<>
struct CatmullRomInterpolation<float> {
    static float Evaluate(float a, float b, float c, float d, float t) {
        return CatmullRom(a, b, c, d, t);
    }
};

template<>
struct CatmullRomInterpolation<float3> {
    static float3 Evaluate(float3 a, float3 b, float3 c, float3 d, float t) {
        return float3(
            CatmullRom(a.x, b.x, c.x, d.x, t),
            CatmullRom(a.y, b.y, c.y, d.y, t),
            CatmullRom(a.z, b.z, c.z, d.z, t)
        );
    }
};

template<>
struct CatmullRomInterpolation<Quaternion> {
    static Quaternion Evaluate(Quaternion a, Quaternion b, Quaternion c, Quaternion d, float t) {
        return Quaternion(
            CatmullRom(a.x, b.x, c.x, d.x, t),
            CatmullRom(a.y, b.y, c.y, d.y, t),
            CatmullRom(a.z, b.z, c.z, d.z, t),
            CatmullRom(a.w, b.w, c.w, d.w, t)
        ).normalized();
    }
};
