#include "utils/types.h"

matrix3x3 Quaternion::toRotationMatrix() const {
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

Quaternion Quaternion::fromRotationMatrix(const matrix3x3& m) {
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

Quaternion Quaternion::slerp(Quaternion a, Quaternion b, float t) {
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

EulerAngles Quaternion::toEuler() const {
    constexpr float PIDIV2 = 1.5707963267948966f;

    EulerAngles e;

    float sinp = 2.0f * (w * x - y * z);
    if (std::abs(sinp) >= 1.0f)
        e.pitch = std::copysign(PIDIV2, sinp);
    else
        e.pitch = std::asin(sinp);

    float siny = 2.0f * (w * y + z * x);
    float cosy = 1.0f - 2.0f * (x * x + y * y);
    e.yaw = std::atan2(siny, cosy);

    float sinr = 2.0f * (w * z + x * y);
    float cosr = 1.0f - 2.0f * (z * z + x * x);
    e.roll = std::atan2(sinr, cosr);

    return e;
}

Quaternion EulerAngles::toQuaternion() const {
    float cp = std::cos(pitch * 0.5f);
    float sp = std::sin(pitch * 0.5f);
    float cy = std::cos(yaw * 0.5f);
    float sy = std::sin(yaw * 0.5f);
    float cr = std::cos(roll * 0.5f);
    float sr = std::sin(roll * 0.5f);

    Quaternion q;
    q.w = cp * cy * cr + sp * sy * sr;
    q.x = sp * cy * cr - cp * sy * sr;
    q.y = cp * sy * cr + sp * cy * sr;
    q.z = cp * cy * sr - sp * sy * cr;
    return q;
}