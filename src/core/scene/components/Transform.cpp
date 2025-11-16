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

#include "LGE/core/scene/components/Transform.h"
#include "LGE/core/scene/GameObject.h"
#include "LGE/core/Log.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <sstream>

namespace LGE {

Transform::Transform()
    : m_Position(0.0f, 0.0f, 0.0f)
    , m_Rotation(0.0f, 0.0f, 0.0f)
    , m_Scale(1.0f, 1.0f, 1.0f)
    , m_LocalMatrixDirty(true)
    , m_WorldMatrixDirty(true)
    , m_Parent(nullptr)
{
}

void Transform::SetPosition(const Math::Vector3& position) {
    m_Position = position;
    m_LocalMatrixDirty = true;
    m_WorldMatrixDirty = true;
    
    // Mark children as dirty
    for (auto& child : m_Children) {
        if (child) {
            child->m_WorldMatrixDirty = true;
        }
    }
}

void Transform::SetPosition(float x, float y, float z) {
    SetPosition(Math::Vector3(x, y, z));
}

Math::Vector3 Transform::GetWorldPosition() const {
    if (m_Parent) {
        Math::Matrix4 worldMatrix = GetWorldMatrix();
        // Extract position from matrix (last column)
        return Math::Vector3(worldMatrix.m[12], worldMatrix.m[13], worldMatrix.m[14]);
    }
    return m_Position;
}

void Transform::SetRotation(const Math::Vector3& rotation) {
    m_Rotation = rotation;
    m_LocalMatrixDirty = true;
    m_WorldMatrixDirty = true;
    
    // Mark children as dirty
    for (auto& child : m_Children) {
        if (child) {
            child->m_WorldMatrixDirty = true;
        }
    }
}

void Transform::SetRotation(float x, float y, float z) {
    SetRotation(Math::Vector3(x, y, z));
}

Math::Vector3 Transform::GetWorldRotation() const {
    // For simplicity, return local rotation
    // In a full implementation, you'd accumulate parent rotations
    if (m_Parent) {
        return m_Parent->GetWorldRotation() + m_Rotation;
    }
    return m_Rotation;
}

void Transform::SetScale(const Math::Vector3& scale) {
    m_Scale = scale;
    m_LocalMatrixDirty = true;
    m_WorldMatrixDirty = true;
    
    // Mark children as dirty
    for (auto& child : m_Children) {
        if (child) {
            child->m_WorldMatrixDirty = true;
        }
    }
}

void Transform::SetScale(float x, float y, float z) {
    SetScale(Math::Vector3(x, y, z));
}

void Transform::SetUniformScale(float scale) {
    SetScale(Math::Vector3(scale, scale, scale));
}

Math::Vector3 Transform::GetWorldScale() const {
    if (m_Parent) {
        Math::Vector3 parentScale = m_Parent->GetWorldScale();
        return Math::Vector3(
            parentScale.x * m_Scale.x,
            parentScale.y * m_Scale.y,
            parentScale.z * m_Scale.z
        );
    }
    return m_Scale;
}

void Transform::UpdateLocalMatrix() {
    if (!m_LocalMatrixDirty) return;
    
    // Build transform matrix: T * R * S
    Math::Matrix4 translation = Math::Matrix4::Translate(m_Position);
    
    // Rotation as Euler angles (in degrees to radians)
    float rx = m_Rotation.x * 3.14159f / 180.0f;
    float ry = m_Rotation.y * 3.14159f / 180.0f;
    float rz = m_Rotation.z * 3.14159f / 180.0f;
    
    Math::Matrix4 rotX = Math::Matrix4::Rotate(rx, Math::Vector3(1.0f, 0.0f, 0.0f));
    Math::Matrix4 rotY = Math::Matrix4::Rotate(ry, Math::Vector3(0.0f, 1.0f, 0.0f));
    Math::Matrix4 rotZ = Math::Matrix4::Rotate(rz, Math::Vector3(0.0f, 0.0f, 1.0f));
    Math::Matrix4 rotation = rotZ * rotY * rotX;
    
    Math::Matrix4 scale = Math::Matrix4::Scale(m_Scale);
    
    m_LocalMatrix = translation * rotation * scale;
    m_LocalMatrixDirty = false;
}

Math::Matrix4 Transform::GetWorldMatrix() const {
    if (m_WorldMatrixDirty) {
        const_cast<Transform*>(this)->UpdateWorldMatrix();
    }
    return m_WorldMatrix;
}

void Transform::UpdateWorldMatrix() {
    UpdateLocalMatrix();
    
    if (m_Parent) {
        m_WorldMatrix = m_Parent->GetWorldMatrix() * m_LocalMatrix;
    } else {
        m_WorldMatrix = m_LocalMatrix;
    }
    
    m_WorldMatrixDirty = false;
}

const Math::Matrix4& Transform::GetLocalMatrix() {
    UpdateLocalMatrix();
    return m_LocalMatrix;
}

void Transform::SetParent(Transform* parent) {
    // Remove from old parent
    if (m_Parent) {
        m_Parent->RemoveChild(this);
    }
    
    // Set new parent
    m_Parent = parent;
    
    if (parent) {
        parent->AddChild(this);
        m_WorldMatrixDirty = true;
    }
}

void Transform::RemoveParent() {
    SetParent(nullptr);
}

void Transform::AddChild(Transform* child) {
    if (child) {
        m_Children.push_back(child);
    }
}

void Transform::RemoveChild(Transform* child) {
    m_Children.erase(
        std::remove_if(m_Children.begin(), m_Children.end(),
            [child](Transform* c) {
                return c == child;
            }),
        m_Children.end()
    );
}

Transform* Transform::GetChild(size_t index) const {
    if (index < m_Children.size()) {
        return m_Children[index];
    }
    return nullptr;
}

void Transform::Translate(const Math::Vector3& translation) {
    SetPosition(m_Position + translation);
}

void Transform::Rotate(const Math::Vector3& rotation) {
    SetRotation(m_Rotation + rotation);
}

void Transform::RotateAround(const Math::Vector3& point, const Math::Vector3& axis, float angle) {
    // Translate to point, rotate, translate back
    Math::Vector3 offset = m_Position - point;
    // Apply rotation (simplified - would need quaternion for proper rotation)
    SetPosition(point + offset);
}

Math::Vector3 Transform::Forward() const {
    Math::Matrix4 worldMatrix = GetWorldMatrix();
    // Extract forward vector from rotation part of matrix (negative Z)
    Math::Vector3 forward(-worldMatrix.m[8], -worldMatrix.m[9], -worldMatrix.m[10]);
    float length = std::sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    if (length > 0.0f) {
        return forward / length;
    }
    return Math::Vector3(0.0f, 0.0f, -1.0f);
}

Math::Vector3 Transform::Right() const {
    Math::Matrix4 worldMatrix = GetWorldMatrix();
    // Extract right vector from rotation part of matrix (X)
    Math::Vector3 right(worldMatrix.m[0], worldMatrix.m[1], worldMatrix.m[2]);
    float length = std::sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
    if (length > 0.0f) {
        return right / length;
    }
    return Math::Vector3(1.0f, 0.0f, 0.0f);
}

Math::Vector3 Transform::Up() const {
    Math::Matrix4 worldMatrix = GetWorldMatrix();
    // Extract up vector from rotation part of matrix (Y)
    Math::Vector3 up(worldMatrix.m[4], worldMatrix.m[5], worldMatrix.m[6]);
    float length = std::sqrt(up.x * up.x + up.y * up.y + up.z * up.z);
    if (length > 0.0f) {
        return up / length;
    }
    return Math::Vector3(0.0f, 1.0f, 0.0f);
}

std::string Transform::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"Transform\",\n";
    json << "  \"position\": {\"x\": " << m_Position.x << ", \"y\": " << m_Position.y << ", \"z\": " << m_Position.z << "},\n";
    json << "  \"rotation\": {\"x\": " << m_Rotation.x << ", \"y\": " << m_Rotation.y << ", \"z\": " << m_Rotation.z << "},\n";
    json << "  \"scale\": {\"x\": " << m_Scale.x << ", \"y\": " << m_Scale.y << ", \"z\": " << m_Scale.z << "}\n";
    json << "}";
    return json.str();
}

void Transform::Deserialize(const std::string& json) {
    try {
        auto j = nlohmann::json::parse(json);
        
        // Parse position
        if (j.contains("position")) {
            auto pos = j["position"];
            if (pos.contains("x") && pos.contains("y") && pos.contains("z")) {
                m_Position = Math::Vector3(pos["x"], pos["y"], pos["z"]);
            }
        }
        
        // Parse rotation
        if (j.contains("rotation")) {
            auto rot = j["rotation"];
            if (rot.contains("x") && rot.contains("y") && rot.contains("z")) {
                m_Rotation = Math::Vector3(rot["x"], rot["y"], rot["z"]);
            }
        }
        
        // Parse scale
        if (j.contains("scale")) {
            auto scale = j["scale"];
            if (scale.contains("x") && scale.contains("y") && scale.contains("z")) {
                m_Scale = Math::Vector3(scale["x"], scale["y"], scale["z"]);
            }
        }
        
        m_LocalMatrixDirty = true;
        m_WorldMatrixDirty = true;
    } catch (const std::exception& e) {
        Log::Error("Transform::Deserialize failed: " + std::string(e.what()));
    }
}

Transform* Transform::Deserialize(const std::string& json, GameObject* owner) {
    auto transform = new Transform();
    transform->OnAttach(owner);
    transform->Deserialize(json);
    return transform;
}

} // namespace LGE

