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

#include <cstdint>

#include "LGE/math/Vector.h"
#include "LGE/math/Matrix.h"

namespace LGE {

class Camera {
public:
    enum class ProjectionType {
        Perspective,
        Orthographic
    };

    Camera();
    Camera(const Math::Vector3& position, const Math::Vector3& target, const Math::Vector3& up);
    ~Camera() = default;

    // Getters
    const Math::Matrix4& GetViewMatrix() const { return m_ViewMatrix; }
    const Math::Matrix4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
    const Math::Matrix4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
    
    Math::Vector3 GetPosition() const { return m_Position; }
    Math::Vector3 GetTarget() const { return m_Target; }
    Math::Vector3 GetUp() const { return m_Up; }
    
    // Projection parameter getters (for shadow system)
    float GetFOV() const { return m_FOV * 180.0f / 3.14159f; } // Return in degrees
    float GetAspectRatio() const { return m_AspectRatio; }
    float GetNearPlane() const { return m_NearPlane; }
    float GetFarPlane() const { return m_FarPlane; }
    ProjectionType GetProjectionType() const { return m_ProjectionType; }

    // Setters
    void SetPosition(const Math::Vector3& position);
    void SetTarget(const Math::Vector3& target);
    void SetUp(const Math::Vector3& up);
    
    // Projection settings
    void SetPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);
    void SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void SetProjectionType(ProjectionType type) { m_ProjectionType = type; }
    
    // Update matrices (call after changing position/target)
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();
    void UpdateViewProjectionMatrix();

    // Camera movement
    void Move(const Math::Vector3& offset);
    void Rotate(float angle, const Math::Vector3& axis);
    void Zoom(float delta);
    void Pan(const Math::Vector3& offset);

private:
    Math::Vector3 m_Position;
    Math::Vector3 m_Target;
    Math::Vector3 m_Up;
    
    Math::Matrix4 m_ViewMatrix;
    Math::Matrix4 m_ProjectionMatrix;
    Math::Matrix4 m_ViewProjectionMatrix;
    
    ProjectionType m_ProjectionType;
    
    // Perspective projection parameters
    float m_FOV;
    float m_AspectRatio;
    float m_NearPlane;
    float m_FarPlane;
    
    // Orthographic projection parameters
    float m_Left, m_Right, m_Bottom, m_Top;
};

} // namespace LGE

