#pragma once
#include <cmath>
#include <array>

#include "utils/debug.h"

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

struct EulerAngles;

struct Quaternion : float4 {
    Quaternion() : float4(0, 0, 0, 1) {}
    Quaternion(float4 vec4) : float4(vec4) {}
    Quaternion(float x, float y, float z, float w) : float4(x, y, z, w) {}

    matrix3x3 toRotationMatrix() const;
    EulerAngles toEuler() const;
    static Quaternion fromRotationMatrix(const matrix3x3& m);
    static Quaternion slerp(Quaternion a, Quaternion b, float t);

    bool operator==(const Quaternion& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
    bool operator!=(const Quaternion& other) const { return !(*this == other); }
};

struct EulerAngles {
    EulerAngles() = default;
    EulerAngles(float yaw, float pitch, float roll) : yaw(yaw), pitch(pitch), roll(roll) {}

    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    Quaternion toQuaternion() const;
};

template<typename T, size_t N>
struct FixedVec {
    std::array<T, N> data{};
    size_t count = 0;

    FixedVec() = default;
    FixedVec(std::initializer_list<T> list) {
        for (const T& val : list)
            push_back(val);
    }

    bool empty() const { return count == 0; }
    size_t size() const { return count; }
    static constexpr size_t capacity() { return N; }

    auto begin() { return data.begin(); }
    auto end() { return data.begin() + count; }
    auto begin() const { return data.begin(); }
    auto end() const { return data.begin() + count; }

    void push_back(const T& val) {
        if (count >= N) {
            LOG_ERROR("FixedVec overflow");
            return;
        }
        data[count++] = val;
    }

    bool try_push_back(const T& val) {
        if (count >= N) return false;
        data[count++] = val;
        return true;
    }

    void clear() { count = 0; }

    T& operator[](size_t i) { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }
};

struct Vertex {
    float3 position;
    float4 color;
};