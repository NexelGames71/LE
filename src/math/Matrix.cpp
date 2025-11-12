/*

------------------------------------------------------------------------------

Luma Engine - A Next-Generation Game Engine

Copyright (c) 2025 Nexel Games. All Rights Reserved.



This source code is part of the Luma Engine project developed by Nexel Games.

Use of this software is governed by the Luma Engine License Agreement.



Unauthorized copying of this file, via any medium, is strictly prohibited.

Distribution of source or binary forms, with or without modification, is

subject to the terms of the Luma Engine License.



For more information, visit: https://nexelgames.com/luma-engine

------------------------------------------------------------------------------

*/

#include "LGE/math/Matrix.h"
#include "LGE/math/Vector.h"
#include <cmath>
#include <cstring>

namespace LGE {
namespace Math {

Matrix4::Matrix4() {
    std::memset(m, 0, 16 * sizeof(float));
}

Matrix4::Matrix4(float diagonal) {
    std::memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = diagonal;
}

Matrix4 Matrix4::Identity() {
    return Matrix4(1.0f);
}

Matrix4 Matrix4::Orthographic(float left, float right, float bottom, float top, float near, float far) {
    Matrix4 result(1.0f);
    
    result.m[0] = 2.0f / (right - left);
    result.m[5] = 2.0f / (top - bottom);
    result.m[10] = -2.0f / (far - near);
    result.m[12] = -(right + left) / (right - left);
    result.m[13] = -(top + bottom) / (top - bottom);
    result.m[14] = -(far + near) / (far - near);
    
    return result;
}

Matrix4 Matrix4::Perspective(float fov, float aspectRatio, float near, float far) {
    Matrix4 result;
    
    float f = 1.0f / std::tan(fov * 0.5f);
    float range = far - near;
    
    result.m[0] = f / aspectRatio;
    result.m[5] = f;
    result.m[10] = -(far + near) / range;
    result.m[11] = -1.0f;
    result.m[14] = -(2.0f * far * near) / range;
    
    return result;
}

Matrix4 Matrix4::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
    Vector3 f = Vector3(center.x - eye.x, center.y - eye.y, center.z - eye.z);
    float length = std::sqrt(f.x * f.x + f.y * f.y + f.z * f.z);
    f = Vector3(f.x / length, f.y / length, f.z / length);
    
    Vector3 s = Vector3(
        f.y * up.z - f.z * up.y,
        f.z * up.x - f.x * up.z,
        f.x * up.y - f.y * up.x
    );
    length = std::sqrt(s.x * s.x + s.y * s.y + s.z * s.z);
    s = Vector3(s.x / length, s.y / length, s.z / length);
    
    Vector3 u = Vector3(
        s.y * f.z - s.z * f.y,
        s.z * f.x - s.x * f.z,
        s.x * f.y - s.y * f.x
    );
    
    Matrix4 result(1.0f);
    result.m[0] = s.x;
    result.m[4] = s.y;
    result.m[8] = s.z;
    result.m[1] = u.x;
    result.m[5] = u.y;
    result.m[9] = u.z;
    result.m[2] = -f.x;
    result.m[6] = -f.y;
    result.m[10] = -f.z;
    result.m[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    result.m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    result.m[14] = (f.x * eye.x + f.y * eye.y + f.z * eye.z);
    
    return result;
}

Matrix4 Matrix4::Translate(const Vector3& translation) {
    Matrix4 result(1.0f);
    result.m[12] = translation.x;
    result.m[13] = translation.y;
    result.m[14] = translation.z;
    return result;
}

Matrix4 Matrix4::Rotate(float angle, const Vector3& axis) {
    Matrix4 result(1.0f);
    
    float c = std::cos(angle);
    float s = std::sin(angle);
    float omc = 1.0f - c;
    
    float x = axis.x, y = axis.y, z = axis.z;
    
    result.m[0] = x * x * omc + c;
    result.m[4] = y * x * omc + z * s;
    result.m[8] = x * z * omc - y * s;
    
    result.m[1] = x * y * omc - z * s;
    result.m[5] = y * y * omc + c;
    result.m[9] = y * z * omc + x * s;
    
    result.m[2] = x * z * omc + y * s;
    result.m[6] = y * z * omc - x * s;
    result.m[10] = z * z * omc + c;
    
    return result;
}

Matrix4 Matrix4::Scale(const Vector3& scale) {
    Matrix4 result(1.0f);
    result.m[0] = scale.x;
    result.m[5] = scale.y;
    result.m[10] = scale.z;
    return result;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i + j * 4] = 0.0f;
            for (int k = 0; k < 4; k++) {
                result.m[i + j * 4] += m[i + k * 4] * other.m[k + j * 4];
            }
        }
    }
    
    return result;
}

Vector4 Matrix4::operator*(const Vector4& vec) const {
    return Vector4(
        m[0] * vec.x + m[4] * vec.y + m[8] * vec.z + m[12] * vec.w,
        m[1] * vec.x + m[5] * vec.y + m[9] * vec.z + m[13] * vec.w,
        m[2] * vec.x + m[6] * vec.y + m[10] * vec.z + m[14] * vec.w,
        m[3] * vec.x + m[7] * vec.y + m[11] * vec.z + m[15] * vec.w
    );
}

} // namespace Math
} // namespace LGE


