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

#include "LGE/rendering/Camera.h"
#include <cmath>

namespace LGE {

Camera::Camera()
    : m_Position(0.0f, 0.0f, 3.0f)
    , m_Target(0.0f, 0.0f, 0.0f)
    , m_Up(0.0f, 1.0f, 0.0f)
    , m_ProjectionType(ProjectionType::Perspective)
    , m_FOV(45.0f * 3.14159f / 180.0f) // 45 degrees in radians
    , m_AspectRatio(16.0f / 9.0f)
    , m_NearPlane(0.1f)
    , m_FarPlane(100.0f)
    , m_Left(-1.0f)
    , m_Right(1.0f)
    , m_Bottom(-1.0f)
    , m_Top(1.0f)
{
    UpdateViewMatrix();
    UpdateProjectionMatrix();
    UpdateViewProjectionMatrix();
}

Camera::Camera(const Math::Vector3& position, const Math::Vector3& target, const Math::Vector3& up)
    : m_Position(position)
    , m_Target(target)
    , m_Up(up)
    , m_ProjectionType(ProjectionType::Perspective)
    , m_FOV(45.0f * 3.14159f / 180.0f)
    , m_AspectRatio(16.0f / 9.0f)
    , m_NearPlane(0.1f)
    , m_FarPlane(100.0f)
    , m_Left(-1.0f)
    , m_Right(1.0f)
    , m_Bottom(-1.0f)
    , m_Top(1.0f)
{
    UpdateViewMatrix();
    UpdateProjectionMatrix();
    UpdateViewProjectionMatrix();
}

void Camera::SetPosition(const Math::Vector3& position) {
    m_Position = position;
    UpdateViewMatrix();
    UpdateViewProjectionMatrix();
}

void Camera::SetTarget(const Math::Vector3& target) {
    m_Target = target;
    UpdateViewMatrix();
    UpdateViewProjectionMatrix();
}

void Camera::SetUp(const Math::Vector3& up) {
    m_Up = up;
    UpdateViewMatrix();
    UpdateViewProjectionMatrix();
}

void Camera::SetPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
    m_ProjectionType = ProjectionType::Perspective;
    m_FOV = fov * 3.14159f / 180.0f; // Convert to radians
    m_AspectRatio = aspectRatio;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    UpdateProjectionMatrix();
    UpdateViewProjectionMatrix();
}

void Camera::SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    m_ProjectionType = ProjectionType::Orthographic;
    m_Left = left;
    m_Right = right;
    m_Bottom = bottom;
    m_Top = top;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    UpdateProjectionMatrix();
    UpdateViewProjectionMatrix();
}

void Camera::UpdateViewMatrix() {
    m_ViewMatrix = Math::Matrix4::LookAt(m_Position, m_Target, m_Up);
}

void Camera::UpdateProjectionMatrix() {
    if (m_ProjectionType == ProjectionType::Perspective) {
        m_ProjectionMatrix = Math::Matrix4::Perspective(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
    } else {
        m_ProjectionMatrix = Math::Matrix4::Orthographic(m_Left, m_Right, m_Bottom, m_Top, m_NearPlane, m_FarPlane);
    }
}

void Camera::UpdateViewProjectionMatrix() {
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::Move(const Math::Vector3& offset) {
    m_Position = m_Position + offset;
    m_Target = m_Target + offset;
    UpdateViewMatrix();
    UpdateViewProjectionMatrix();
}

void Camera::Rotate(float angle, const Math::Vector3& axis) {
    // Simple rotation implementation
    // For a more complete implementation, you'd want to use quaternions
    Math::Vector3 direction = Math::Vector3(
        m_Target.x - m_Position.x,
        m_Target.y - m_Position.y,
        m_Target.z - m_Position.z
    );
    
    Math::Matrix4 rotation = Math::Matrix4::Rotate(angle, axis);
    Math::Vector4 rotatedDir = rotation * Math::Vector4(direction.x, direction.y, direction.z, 0.0f);
    
    m_Target = Math::Vector3(
        m_Position.x + rotatedDir.x,
        m_Position.y + rotatedDir.y,
        m_Position.z + rotatedDir.z
    );
    
    UpdateViewMatrix();
    UpdateViewProjectionMatrix();
}

void Camera::Zoom(float delta) {
    Math::Vector3 position = m_Position;
    Math::Vector3 target = m_Target;
    Math::Vector3 direction = Math::Vector3(
        target.x - position.x,
        target.y - position.y,
        target.z - position.z
    );
    
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (distance > 0.0f) {
        direction = Math::Vector3(direction.x / distance, direction.y / distance, direction.z / distance);
    }
    
    // Move camera closer or farther from target
    Math::Vector3 newPosition = position + direction * delta;
    
    // Don't zoom past the target
    Math::Vector3 newDirection = Math::Vector3(
        target.x - newPosition.x,
        target.y - newPosition.y,
        target.z - newPosition.z
    );
    float newDistance = std::sqrt(newDirection.x * newDirection.x + newDirection.y * newDirection.y + newDirection.z * newDirection.z);
    
    if (newDistance > 0.1f && delta < 0.0f || delta > 0.0f) {
        m_Position = newPosition;
        UpdateViewMatrix();
        UpdateViewProjectionMatrix();
    }
}

void Camera::Pan(const Math::Vector3& offset) {
    Math::Vector3 position = m_Position;
    Math::Vector3 target = m_Target;
    Math::Vector3 direction = Math::Vector3(
        target.x - position.x,
        target.y - position.y,
        target.z - position.z
    );
    
    // Normalize direction
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (length > 0.0f) {
        direction = Math::Vector3(direction.x / length, direction.y / length, direction.z / length);
    }
    
    // Calculate right vector
    Math::Vector3 up = m_Up;
    Math::Vector3 right = Math::Vector3(
        direction.y * up.z - direction.z * up.y,
        direction.z * up.x - direction.x * up.z,
        direction.x * up.y - direction.y * up.x
    );
    
    // Normalize right
    length = std::sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
    if (length > 0.0f) {
        right = Math::Vector3(right.x / length, right.y / length, right.z / length);
    }
    
    // Pan in the plane perpendicular to the view direction
    m_Position = m_Position + offset;
    m_Target = m_Target + offset;
    
    UpdateViewMatrix();
    UpdateViewProjectionMatrix();
}

} // namespace LGE

