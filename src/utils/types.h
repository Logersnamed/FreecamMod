#pragma once
#include <cmath>

struct float2 {
    float x, y;

    constexpr float2() : x(0), y(0) {}
    constexpr float2(float t) : x(t), y(t) {}
    constexpr float2(float x, float y) : x(x), y(y) {}

    float2& operator+=(const float2& v) { x += v.x; y += v.y; return *this; }
    float2& operator-=(const float2& v) { x -= v.x; y -= v.y; return *this; }
    float2& operator*=(float s) { x *= s; y *= s; return *this; }
    float2& operator/=(float s) { x /= s; y /= s; return *this; }

    constexpr float2 operator+(const float2& v) const { return { x + v.x, y + v.y }; }
    constexpr float2 operator-(const float2& v) const { return { x - v.x, y - v.y }; }
    constexpr float2 operator*(float s) const { return { x * s, y * s }; }
    constexpr float2 operator/(float s) const { return { x / s, y / s }; }

    constexpr float2 operator-() const { return { -x, -y }; }

    bool operator==(const float2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const float2& v) const { return !(*this == v); }

    inline float length() const { return std::sqrt(x * x + y * y); }
    inline float lengthSquared() const { return x * x + y * y; }

    float2 normalized() const {
        float l = length();
        return l == 0.0f ? float2(0) : *this / l;
    }

    float2 rotated(float sinAngle, float cosAngle) const {
        return {
            x * cosAngle - y * sinAngle,
            x * sinAngle + y * cosAngle
        };
    }

    static float dot(const float2& a, const float2& b) { return a.x * b.x + a.y * b.y; }
};

struct int2 {
    int x, y;

    constexpr int2() : x(0), y(0) {}
    constexpr int2(int t) : x(t), y(t) {}
    constexpr int2(int x, int y) : x(x), y(y) {}

    constexpr explicit operator float2() const {
        return { static_cast<float>(x), static_cast<float>(y) };
    }
};

struct float3 {
    float x, y, z;

    constexpr float3() : x(0), y(0), z(0) {}
    constexpr float3(float t) : x(t), y(t), z(t) {}
    constexpr float3(float x, float y, float z) : x(x), y(y), z(z) {}

    constexpr float3 operator+(const float3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    constexpr float3 operator-(const float3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    constexpr float3 operator*(float s) const { return { x * s, y * s, z * s }; }
    constexpr float3 operator/(float s) const { return { x / s, y / s, z / s }; }

    float3& operator+=(const float3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    float3& operator-=(const float3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    float3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    float3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

    bool operator==(const float3& other) const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(const float3& other) const { return !(*this == other); }

    inline float length() const { return std::sqrt(x * x + y * y + z * z); }
    inline float lengthSquared() const { return x * x + y * y + z * z; }

    float3 normalized() const {
        float l = length();
        return l == 0.0f ? float3(0) : *this / l;
    }

    static float dot(const float3& a, const float3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static float3 cross(const float3& a, const float3& b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    static float3 forward() { return { 0, 0, 1 }; }
    static float3 back() { return { 0, 0, -1 }; }
    static float3 right() { return { 1, 0, 0 }; }
    static float3 left() { return { -1, 0, 0 }; }
    static float3 up() { return { 0, 1, 0 }; }
    static float3 down() { return { 0, -1, 0 }; }
};

struct float3_ref {
    float& x;
    float& y;
    float& z;

    constexpr float3_ref(float& x, float& y, float& z) : x(x), y(y), z(z) {}

    operator float3() const { return { x, y, z }; }

    float3_ref& operator=(const float3& v) { x = v.x; y = v.y; z = v.z; return *this; }
    float3_ref& operator=(const float3_ref& v) { x = v.x; y = v.y; z = v.z; return *this; }
    float3_ref& operator+=(const float3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    float3_ref& operator+=(const float3_ref& v) { x += v.x; y += v.y; z += v.z; return *this; }
};

struct float4 {
    float x, y, z, w;

    constexpr float4() : x(0), y(0), z(0), w(0) {}
    constexpr float4(float t) : x(t), y(t), z(t), w(t) {}
    constexpr float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    constexpr float4(float3 vec3, float w) : x(vec3.x), y(vec3.y), z(vec3.z), w(w) {}

    constexpr float3 xyz() const { return { x,y,z }; }
    inline float length() const { return std::sqrt(x * x + y * y + z * z + w * w); }

    float4 normalized() const {
        float l = length();
        return l == 0.0f ? float4(0) : *this / l;
    }

    constexpr float4 operator/(float s) const { return { x / s, y / s, z / s, w / s }; }

    bool operator==(const float4& other) const {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }
};

struct matrix3x3 {
    float3 c0, c1, c2;

    constexpr matrix3x3()
        : c0(1, 0, 0),
        c1(0, 1, 0),
        c2(0, 0, 1) {
    }

    constexpr matrix3x3(const float3& c0, const float3& c1, const float3& c2)
        : c0(c0), c1(c1), c2(c2) {}

    static matrix3x3 identity() { return {}; }

    bool operator==(const matrix3x3& other) const { return c0 == other.c0 && c1 == other.c1 && c2 == other.c2; }
    bool operator!=(const matrix3x3& other) const { return !(*this == other); }
};

struct matrix3x3_ref {
    float3_ref c0;
    float3_ref c1;
    float3_ref c2;

    operator matrix3x3() const { return { (float3)c0, (float3)c1, (float3)c2 }; }

    matrix3x3_ref& operator=(const matrix3x3& m) { c0 = m.c0; c1 = m.c1; c2 = m.c2; return *this; }
};

struct matrix4x4 {
    float4 c0, c1, c2, c3;

    constexpr matrix4x4()
        : c0(1, 0, 0, 0),
        c1(0, 1, 0, 0),
        c2(0, 0, 1, 0),
        c3(0, 0, 0, 1) {
    }

    static matrix4x4 identity() { return matrix4x4(); }

    float3_ref position() { return { c3.x, c3.y, c3.z }; }
    float3 position() const { return { c3.x, c3.y, c3.z }; }

    matrix3x3_ref rotation() { return { {c0.x, c0.y, c0.z}, {c1.x, c1.y, c1.z}, {c2.x, c2.y, c2.z} }; }
    matrix3x3 rotation() const { return { {c0.x, c0.y, c0.z}, {c1.x, c1.y, c1.z}, {c2.x, c2.y, c2.z} }; }

    bool operator==(const matrix4x4& other) const { return c0 == other.c0 && c1 == other.c1 && c2 == other.c2 && c3 == other.c3; }
    bool operator!=(const matrix4x4& other) const { return !(*this == other); }
};

struct Quaternion : float4 {
    Quaternion() : float4(0, 0, 0, 1) {}
    Quaternion(float4 vec4) : float4(vec4) {}
    Quaternion(float x, float y, float z, float w) : float4(x, y, z, w) {}

    matrix3x3 toRotationMatrix() const {
        float xx = x * x;
        float yy = y * y;
        float zz = z * z;
        float xy = x * y;
        float xz = x * z;
        float yz = y * z;
        float wx = w * x;
        float wy = w * y;
        float wz = w * z;

        return matrix3x3(
            float3(
                1.0f - 2.0f * (yy + zz),
                2.0f * (xy - wz),
                2.0f * (xz + wy)
            ),
            float3(
                2.0f * (xy + wz),
                1.0f - 2.0f * (xx + zz),
                2.0f * (yz - wx)
            ),
            float3(
                2.0f * (xz - wy),
                2.0f * (yz + wx),
                1.0f - 2.0f * (xx + yy)
            )
        );
    }

    static Quaternion fromRotationMatrix(const matrix3x3& m) {
        Quaternion quaternion;
        float trace = m.c0.x + m.c1.y + m.c2.z;

        if (trace > 0.0f) {
            float s = std::sqrt(trace + 1.0f) * 2.0f;
            quaternion.w = 0.25f * s;
            quaternion.x = (m.c2.y - m.c1.z) / s;
            quaternion.y = (m.c0.z - m.c2.x) / s;
            quaternion.z = (m.c1.x - m.c0.y) / s;
        }
        else {
            if (m.c0.x > m.c1.y && m.c0.x > m.c2.z) {
                float s = std::sqrt(1.0f + m.c0.x - m.c1.y - m.c2.z) * 2.0f;
                quaternion.w = (m.c2.y - m.c1.z) / s;
                quaternion.x = 0.25f * s;
                quaternion.y = (m.c0.y + m.c1.x) / s;
                quaternion.z = (m.c0.z + m.c2.x) / s;
            }
            else if (m.c1.y > m.c2.z) {
                float s = std::sqrt(1.0f + m.c1.y - m.c0.x - m.c2.z) * 2.0f;
                quaternion.w = (m.c0.z - m.c2.x) / s;
                quaternion.x = (m.c0.y + m.c1.x) / s;
                quaternion.y = 0.25f * s;
                quaternion.z = (m.c1.z + m.c2.y) / s;
            }
            else {
                float s = std::sqrt(1.0f + m.c2.z - m.c0.x - m.c1.y) * 2.0f;
                quaternion.w = (m.c1.x - m.c0.y) / s;
                quaternion.x = (m.c0.z + m.c2.x) / s;
                quaternion.y = (m.c1.z + m.c2.y) / s;
                quaternion.z = 0.25f * s;
            }
        }

        return Quaternion(quaternion.normalized());
    }

    static Quaternion slerp(Quaternion a, Quaternion b, float t) {
        float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

        if (dot < 0.0f) {
            b.x = -b.x;
            b.y = -b.y;
            b.z = -b.z;
            b.w = -b.w;
            dot = -dot;
        }

        const float DOT_THRESHOLD = 0.9995f;
        if (dot > DOT_THRESHOLD) {
            Quaternion result(
                a.x + t * (b.x - a.x),
                a.y + t * (b.y - a.y),
                a.z + t * (b.z - a.z),
                a.w + t * (b.w - a.w)
            );
            return Quaternion(result.normalized());
        }

        float theta0 = std::acos(dot);
        float theta = theta0 * t;

        float sin_theta = std::sin(theta);
        float sin_theta0 = std::sin(theta0);

        float s0 = std::cos(theta) - dot * sin_theta / sin_theta0;
        float s1 = sin_theta / sin_theta0;

        return Quaternion(
            (a.x * s0) + (b.x * s1),
            (a.y * s0) + (b.y * s1),
            (a.z * s0) + (b.z * s1),
            (a.w * s0) + (b.w * s1)
        );
    }

    bool operator==(const Quaternion& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
    bool operator!=(const Quaternion& other) const { return !(*this == other); }
};

struct EulerAngles {
    EulerAngles() = default;
    EulerAngles(float yaw, float pitch, float roll) : yaw(yaw), pitch(pitch), roll(roll) {}

    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
};