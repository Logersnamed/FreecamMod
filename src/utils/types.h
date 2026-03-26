#pragma once
#include <cmath>

struct float2 {
    float x, y;

    constexpr float2() : x(0), y(0) {}
    constexpr float2(float t) : x(t), y(t) {}
    constexpr float2(float x, float y) : x(x), y(y) {}
};

struct int2 {
    int x, y;

    constexpr int2() : x(0), y(0) {}
    constexpr int2(int t) : x(t), y(t) {}
    constexpr int2(int x, int y) : x(x), y(y) {}

    float2 rotate(float sin, float cos) const {
        return { 
             x * cos + y * sin,
            -x * sin + y * cos 
        };
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

    bool operator==(const float3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const float3& other) const {
        return !(*this == other);
    }
};

struct float3_ref {
    float& x;
    float& y;
    float& z;

    operator float3() const { return { x, y, z }; }

    float3_ref& operator=(const float3& v) {
        x = v.x; y = v.y; z = v.z;
        return *this;
    }

    float3_ref& operator+=(const float3& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
};

struct float4 {
    float x, y, z, w;

    constexpr float4() : x(0), y(0), z(0), w(0) {}
    constexpr float4(float t) : x(t), y(t), z(t), w(t) {}
    constexpr float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    constexpr float4(float3 vec3, float w) : x(vec3.x), y(vec3.y), z(vec3.z), w(w) {}

    constexpr float3 xyz() const { return { x,y,z }; }

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
        : c0(c0), c1(c1), c2(c2) {
    }

    static matrix3x3 identity() { return {}; }

    bool operator==(const matrix3x3& other) const {
        return c0 == other.c0 && c1 == other.c1 && c2 == other.c2;
    }

    bool operator!=(const matrix3x3& other) const {
        return !(*this == other);
    }
};

struct matrix3x3_ref {
    float3_ref c0;
    float3_ref c1;
    float3_ref c2;

    operator matrix3x3() const { return { (float3)c0, (float3)c1, (float3)c2 }; }

    matrix3x3_ref& operator=(const matrix3x3& m) {
        c0 = m.c0;
        c1 = m.c1;
        c2 = m.c2;
        return *this;
    }
};

struct matrix4x4 {
    float4 c0, c1, c2, c3;

    constexpr matrix4x4()
        :   c0(1, 0, 0, 0),
            c1(0, 1, 0, 0),
            c2(0, 0, 1, 0),
            c3(0, 0, 0, 1) {}

    static matrix4x4 identity() { return matrix4x4(); }

    float3_ref position() {
        return { c3.x, c3.y, c3.z };
    }

    float3 position() const {
        return { c3.x, c3.y, c3.z };
    }

    matrix3x3_ref rotation() {
        return { {c0.x, c0.y, c0.z}, {c1.x, c1.y, c1.z}, {c2.x, c2.y, c2.z} };
    }

    matrix3x3 rotation() const {
        return matrix3x3(
            { c0.x, c0.y, c0.z },
            { c1.x, c1.y, c1.z },
            { c2.x, c2.y, c2.z }
        );
    }

    bool operator==(const matrix4x4& other) const {
        return c0 == other.c0 && c1 == other.c1 && c2 == other.c2 && c3 == other.c3;
    }

    bool operator!=(const matrix4x4& other) {
        return !(*this == other);
    }
};