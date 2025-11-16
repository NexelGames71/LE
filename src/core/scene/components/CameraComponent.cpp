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

#include "LGE/core/scene/components/CameraComponent.h"
#include "LGE/rendering/Camera.h"
#include "LGE/core/scene/components/Transform.h"
#include "LGE/core/scene/GameObject.h"
#include <sstream>
#include <cmath>

namespace LGE {

CameraComponent* CameraComponent::s_MainCamera = nullptr;

CameraComponent::CameraComponent()
    : m_ProjectionType(ProjectionType::Perspective)
    , m_FieldOfView(60.0f)
    , m_OrthographicSize(5.0f)
    , m_NearPlane(0.1f)
    , m_FarPlane(1000.0f)
    , m_ClearColor(0.1f, 0.1f, 0.1f, 1.0f)
    , m_AspectRatio(16.0f / 9.0f)
{
    m_Camera = std::make_unique<Camera>();
    UpdateProjectionSettings();
}

CameraComponent::~CameraComponent() {
    if (s_MainCamera == this) {
        s_MainCamera = nullptr;
    }
}

void CameraComponent::SetProjectionType(ProjectionType type) {
    m_ProjectionType = type;
    UpdateProjectionSettings();
}

CameraComponent::ProjectionType CameraComponent::GetProjectionType() const {
    return m_ProjectionType;
}

void CameraComponent::SetFieldOfView(float fov) {
    m_FieldOfView = fov;
    if (m_ProjectionType == ProjectionType::Perspective) {
        UpdateProjectionSettings();
    }
}

float CameraComponent::GetFieldOfView() const {
    return m_FieldOfView;
}

void CameraComponent::SetOrthographicSize(float size) {
    m_OrthographicSize = size;
    if (m_ProjectionType == ProjectionType::Orthographic) {
        UpdateProjectionSettings();
    }
}

float CameraComponent::GetOrthographicSize() const {
    return m_OrthographicSize;
}

void CameraComponent::SetNearClipPlane(float nearPlane) {
    m_NearPlane = nearPlane;
    UpdateProjectionSettings();
}

float CameraComponent::GetNearClipPlane() const {
    return m_NearPlane;
}

void CameraComponent::SetFarClipPlane(float farPlane) {
    m_FarPlane = farPlane;
    UpdateProjectionSettings();
}

float CameraComponent::GetFarClipPlane() const {
    return m_FarPlane;
}

void CameraComponent::SetClearColor(const Math::Vector4& color) {
    m_ClearColor = color;
}

Math::Vector4 CameraComponent::GetClearColor() const {
    return m_ClearColor;
}

void CameraComponent::UpdateProjectionSettings() {
    if (!m_Camera) return;
    
    // Update aspect ratio from viewport if available (default to 16:9)
    // In a full implementation, this would get from the rendering context
    if (m_AspectRatio <= 0.0f) {
        m_AspectRatio = 16.0f / 9.0f;
    }
    
    if (m_ProjectionType == ProjectionType::Perspective) {
        float fovRad = m_FieldOfView * 3.14159f / 180.0f;
        m_Camera->SetPerspective(fovRad, m_AspectRatio, m_NearPlane, m_FarPlane);
    } else {
        float halfSize = m_OrthographicSize * 0.5f;
        float aspect = m_AspectRatio;
        m_Camera->SetOrthographic(-halfSize * aspect, halfSize * aspect, -halfSize, halfSize, m_NearPlane, m_FarPlane);
    }
}

void CameraComponent::SetAspectRatio(float aspectRatio) {
    if (m_AspectRatio != aspectRatio) {
        m_AspectRatio = aspectRatio;
        UpdateProjectionSettings();
    }
}

void CameraComponent::UpdateCamera() {
    if (!m_Camera || !m_Owner) return;
    
    Transform* transform = m_Owner->GetTransform();
    if (!transform) return;
    
    Math::Vector3 position = transform->GetWorldPosition();
    Math::Vector3 forward = transform->Forward();
    Math::Vector3 up = transform->Up();
    
    m_Camera->SetPosition(position);
    m_Camera->SetTarget(position + forward);
    m_Camera->SetUp(up);
    m_Camera->UpdateViewMatrix();
    m_Camera->UpdateViewProjectionMatrix();
}

void CameraComponent::Awake() {
    UpdateCamera();
    
    // Set as main camera if none exists
    if (!s_MainCamera) {
        s_MainCamera = this;
    }
}

void CameraComponent::Update(float deltaTime) {
    UpdateCamera();
}

std::string CameraComponent::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"CameraComponent\",\n";
    json << "  \"projectionType\": " << static_cast<int>(m_ProjectionType) << ",\n";
    json << "  \"fieldOfView\": " << m_FieldOfView << ",\n";
    json << "  \"orthographicSize\": " << m_OrthographicSize << ",\n";
    json << "  \"nearPlane\": " << m_NearPlane << ",\n";
    json << "  \"farPlane\": " << m_FarPlane << ",\n";
    json << "  \"clearColor\": {\"x\": " << m_ClearColor.x << ", \"y\": " << m_ClearColor.y << ", \"z\": " << m_ClearColor.z << ", \"w\": " << m_ClearColor.w << "}\n";
    json << "}";
    return json.str();
}

void CameraComponent::Deserialize(const std::string& json) {
    // Manual JSON parsing (simplified)
    UpdateProjectionSettings();
}

} // namespace LGE

