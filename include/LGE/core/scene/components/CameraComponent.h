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

#include "LGE/core/scene/Component.h"
#include "LGE/core/scene/ComponentFactory.h"
#include "LGE/math/Vector.h"
#include <memory>

namespace LGE {

// Forward declaration
class Camera;

// Camera component - wraps the rendering Camera class
class CameraComponent : public Component {
public:
    enum class ProjectionType {
        Perspective,
        Orthographic
    };
    
    CameraComponent();
    ~CameraComponent() override;
    
    // Component interface
    const char* GetTypeName() const override { return "CameraComponent"; }
    bool IsUnique() const override { return true; }  // Only one camera component per GameObject
    
    // Projection type
    void SetProjectionType(ProjectionType type);
    ProjectionType GetProjectionType() const;
    
    // Perspective settings
    void SetFieldOfView(float fov);
    float GetFieldOfView() const;
    
    // Orthographic settings
    void SetOrthographicSize(float size);
    float GetOrthographicSize() const;
    
    // Clipping planes
    void SetNearClipPlane(float nearPlane);
    float GetNearClipPlane() const;
    
    void SetFarClipPlane(float farPlane);
    float GetFarClipPlane() const;
    
    // Clear color
    void SetClearColor(const Math::Vector4& color);
    Math::Vector4 GetClearColor() const;
    
    // Get underlying Camera object (for rendering)
    Camera* GetCamera() const { return m_Camera.get(); }
    
    // Update camera from transform
    void UpdateCamera();
    
    // Set aspect ratio (called by rendering system)
    void SetAspectRatio(float aspectRatio);
    float GetAspectRatio() const { return m_AspectRatio; }
    
    // Lifecycle
    void Awake() override;
    void Update(float deltaTime) override;
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;
    
    // Static main camera management
    static CameraComponent* GetMainCamera() { return s_MainCamera; }
    static void SetMainCamera(CameraComponent* camera) { s_MainCamera = camera; }

private:
    void UpdateProjectionSettings();
    
private:
    std::unique_ptr<Camera> m_Camera;
    ProjectionType m_ProjectionType;
    float m_FieldOfView;
    float m_OrthographicSize;
    float m_NearPlane;
    float m_FarPlane;
    Math::Vector4 m_ClearColor;
    float m_AspectRatio;
    
    static CameraComponent* s_MainCamera;
};

REGISTER_COMPONENT(CameraComponent)

} // namespace LGE

