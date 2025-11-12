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

#pragma once

namespace LGE {
namespace Math {

// Forward declarations
struct Vector3;
struct Vector4;

struct Matrix4 {
    float m[16]; // Column-major order

    Matrix4();
    Matrix4(float diagonal);
    
    static Matrix4 Identity();
    static Matrix4 Orthographic(float left, float right, float bottom, float top, float near, float far);
    static Matrix4 Perspective(float fov, float aspectRatio, float near, float far);
    static Matrix4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);
    static Matrix4 Translate(const Vector3& translation);
    static Matrix4 Rotate(float angle, const Vector3& axis);
    static Matrix4 Scale(const Vector3& scale);

    Matrix4 operator*(const Matrix4& other) const;
    Vector4 operator*(const Vector4& vec) const;

    float* GetData() { return m; }
    const float* GetData() const { return m; }
};

} // namespace Math
} // namespace LGE


