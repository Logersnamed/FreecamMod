#pragma once
#include <cmath>

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

    inline float length() const { return std::sqrt(x * x + y * y + z * z); }
    inline float lengthSquared() const { return x * x + y * y + z * z; }

    float3 normalized() const {
        float l = length();
        if (l == 0.0f) return float3(0);
        return *this / l;
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

struct float4 {
    float x, y, z, w;

    constexpr float4() : x(0), y(0), z(0), w(0) {}
    constexpr float4(float t) : x(t), y(t), z(t), w(t) {}
    constexpr float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    constexpr float4(float3 vec3, float w) : x(vec3.x), y(vec3.y), z(vec3.z), w(w) {}

    constexpr float3 xyz() const { return { x,y,z }; }
};

struct matrix4x4 {
    float4 c0, c1, c2, c3;

    constexpr matrix4x4()
        :   c0(1, 0, 0, 0),
            c1(0, 1, 0, 0),
            c2(0, 0, 1, 0),
            c3(0, 0, 0, 1) {}

    static matrix4x4 identity() { return matrix4x4(); }

    inline float3 transform_vector(const matrix4x4& m, const float3& v) {
        return {
            m.c0.x * v.x + m.c1.x * v.y + m.c2.x * v.z,
            m.c0.y * v.x + m.c1.y * v.y + m.c2.y * v.z,
            m.c0.z * v.x + m.c1.z * v.y + m.c2.z * v.z
        };
    }

    float3& position() {
        return *reinterpret_cast<float3*>(&c3);
    }

    const float3& position() const {
        return *reinterpret_cast<const float3*>(&c3);
    }
};