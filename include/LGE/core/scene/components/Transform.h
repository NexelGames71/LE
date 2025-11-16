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
#include "LGE/math/Vector.h"
#include "LGE/math/Matrix.h"
#include <memory>
#include <vector>
#include <algorithm>

namespace LGE {

class Transform : public Component {
public:
    Transform();
    ~Transform() override = default;
    
    // Position
    void SetPosition(const Math::Vector3& position);
    void SetPosition(float x, float y, float z);
    Math::Vector3 GetPosition() const { return m_Position; }
    Math::Vector3 GetWorldPosition() const;
    
    // Rotation (Euler angles in degrees)
    void SetRotation(const Math::Vector3& rotation);
    void SetRotation(float x, float y, float z);
    Math::Vector3 GetRotation() const { return m_Rotation; }
    Math::Vector3 GetWorldRotation() const;
    
    // Scale
    void SetScale(const Math::Vector3& scale);
    void SetScale(float x, float y, float z);
    void SetUniformScale(float scale);
    Math::Vector3 GetScale() const { return m_Scale; }
    Math::Vector3 GetWorldScale() const;
    
    // Transform matrix
    const Math::Matrix4& GetLocalMatrix();
    Math::Matrix4 GetWorldMatrix() const;
    
    // Hierarchy (managed by GameObject, these are for internal use)
    void SetParent(Transform* parent);
    Transform* GetParent() const { return m_Parent; }
    void RemoveParent();
    
    const std::vector<Transform*>& GetChildren() const { return m_Children; }
    Transform* GetChild(size_t index) const;
    size_t GetChildCount() const { return m_Children.size(); }
    
    // Transform operations
    void Translate(const Math::Vector3& translation);
    void Rotate(const Math::Vector3& rotation);
    void RotateAround(const Math::Vector3& point, const Math::Vector3& axis, float angle);
    
    // Direction vectors
    Math::Vector3 Forward() const;
    Math::Vector3 Right() const;
    Math::Vector3 Up() const;
    
    // Component interface
    const char* GetTypeName() const override { return "Transform"; }
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;
    
    static Transform* Deserialize(const std::string& json, GameObject* owner);

private:
    void UpdateLocalMatrix();
    void UpdateWorldMatrix();
    void AddChild(Transform* child);
    void RemoveChild(Transform* child);
    
    Math::Vector3 m_Position;
    Math::Vector3 m_Rotation;  // Euler angles in degrees
    Math::Vector3 m_Scale;
    
    Math::Matrix4 m_LocalMatrix;
    Math::Matrix4 m_WorldMatrix;
    bool m_LocalMatrixDirty;
    bool m_WorldMatrixDirty;
    
    // Hierarchy (raw pointers - GameObject manages lifetime)
    Transform* m_Parent;
    std::vector<Transform*> m_Children;
};

} // namespace LGE

