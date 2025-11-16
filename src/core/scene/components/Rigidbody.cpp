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

#include "LGE/core/scene/components/Rigidbody.h"
#include "LGE/core/scene/components/Transform.h"
#include "LGE/core/scene/GameObject.h"
#include <sstream>
#include <algorithm>

namespace LGE {

const Math::Vector3 Rigidbody::s_Gravity(0.0f, -9.81f, 0.0f);

Rigidbody::Rigidbody()
    : m_Mass(1.0f)
    , m_Drag(0.0f)
    , m_AngularDrag(0.05f)
    , m_UseGravity(true)
    , m_Velocity(0.0f, 0.0f, 0.0f)
    , m_AngularVelocity(0.0f, 0.0f, 0.0f)
    , m_FreezePositionX(false)
    , m_FreezePositionY(false)
    , m_FreezePositionZ(false)
    , m_FreezeRotationX(false)
    , m_FreezeRotationY(false)
    , m_FreezeRotationZ(false)
{
}

void Rigidbody::SetMass(float mass) {
    m_Mass = std::max(0.01f, mass);  // Minimum mass
}

void Rigidbody::SetDrag(float drag) {
    m_Drag = std::max(0.0f, drag);
}

void Rigidbody::SetAngularDrag(float angularDrag) {
    m_AngularDrag = std::max(0.0f, angularDrag);
}

void Rigidbody::SetUseGravity(bool useGravity) {
    m_UseGravity = useGravity;
}

void Rigidbody::SetVelocity(const Math::Vector3& velocity) {
    m_Velocity = velocity;
    if (m_FreezePositionX) m_Velocity.x = 0.0f;
    if (m_FreezePositionY) m_Velocity.y = 0.0f;
    if (m_FreezePositionZ) m_Velocity.z = 0.0f;
}

void Rigidbody::SetAngularVelocity(const Math::Vector3& angularVelocity) {
    m_AngularVelocity = angularVelocity;
    if (m_FreezeRotationX) m_AngularVelocity.x = 0.0f;
    if (m_FreezeRotationY) m_AngularVelocity.y = 0.0f;
    if (m_FreezeRotationZ) m_AngularVelocity.z = 0.0f;
}

void Rigidbody::AddForce(const Math::Vector3& force, bool impulse) {
    if (impulse) {
        // Impulse: directly affects velocity
        Math::Vector3 deltaVelocity = force / m_Mass;
        SetVelocity(m_Velocity + deltaVelocity);
    } else {
        // Force: accumulates for physics update
        m_AccumulatedForce = m_AccumulatedForce + force;
    }
}

void Rigidbody::AddTorque(const Math::Vector3& torque, bool impulse) {
    if (impulse) {
        // Simplified: directly affects angular velocity
        SetAngularVelocity(m_AngularVelocity + torque);
    } else {
        // Torque: accumulates for physics update
        m_AccumulatedTorque = m_AccumulatedTorque + torque;
    }
}

void Rigidbody::SetFreezePosition(bool x, bool y, bool z) {
    m_FreezePositionX = x;
    m_FreezePositionY = y;
    m_FreezePositionZ = z;
    // Clear frozen axes
    if (x) m_Velocity.x = 0.0f;
    if (y) m_Velocity.y = 0.0f;
    if (z) m_Velocity.z = 0.0f;
}

void Rigidbody::SetFreezeRotation(bool x, bool y, bool z) {
    m_FreezeRotationX = x;
    m_FreezeRotationY = y;
    m_FreezeRotationZ = z;
    // Clear frozen axes
    if (x) m_AngularVelocity.x = 0.0f;
    if (y) m_AngularVelocity.y = 0.0f;
    if (z) m_AngularVelocity.z = 0.0f;
}

void Rigidbody::PhysicsUpdate(float fixedDeltaTime) {
    if (!m_Owner || m_Mass <= 0.0f) return;
    
    Transform* transform = m_Owner->GetTransform();
    if (!transform) return;
    
    // Apply gravity
    if (m_UseGravity) {
        m_AccumulatedForce = m_AccumulatedForce + (s_Gravity * m_Mass);
    }
    
    // Apply forces to velocity
    Math::Vector3 acceleration = m_AccumulatedForce / m_Mass;
    m_Velocity = m_Velocity + (acceleration * fixedDeltaTime);
    
    // Apply drag
    if (m_Drag > 0.0f) {
        float dragFactor = 1.0f - (m_Drag * fixedDeltaTime);
        dragFactor = std::max(0.0f, std::min(1.0f, dragFactor));
        m_Velocity = m_Velocity * dragFactor;
    }
    
    // Apply constraints
    if (m_FreezePositionX) m_Velocity.x = 0.0f;
    if (m_FreezePositionY) m_Velocity.y = 0.0f;
    if (m_FreezePositionZ) m_Velocity.z = 0.0f;
    
    // Update position
    Math::Vector3 position = transform->GetPosition();
    position = position + (m_Velocity * fixedDeltaTime);
    transform->SetPosition(position);
    
    // Apply angular drag
    if (m_AngularDrag > 0.0f) {
        float angularDragFactor = 1.0f - (m_AngularDrag * fixedDeltaTime);
        angularDragFactor = std::max(0.0f, std::min(1.0f, angularDragFactor));
        m_AngularVelocity = m_AngularVelocity * angularDragFactor;
    }
    
    // Apply rotation constraints
    if (m_FreezeRotationX) m_AngularVelocity.x = 0.0f;
    if (m_FreezeRotationY) m_AngularVelocity.y = 0.0f;
    if (m_FreezeRotationZ) m_AngularVelocity.z = 0.0f;
    
    // Update rotation (simplified - would need quaternions for proper rotation)
    Math::Vector3 rotation = transform->GetRotation();
    rotation = rotation + (m_AngularVelocity * fixedDeltaTime);
    transform->SetRotation(rotation);
    
    // Clear accumulated forces
    m_AccumulatedForce = Math::Vector3(0.0f, 0.0f, 0.0f);
    m_AccumulatedTorque = Math::Vector3(0.0f, 0.0f, 0.0f);
}

std::string Rigidbody::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"Rigidbody\",\n";
    json << "  \"mass\": " << m_Mass << ",\n";
    json << "  \"drag\": " << m_Drag << ",\n";
    json << "  \"angularDrag\": " << m_AngularDrag << ",\n";
    json << "  \"useGravity\": " << (m_UseGravity ? "true" : "false") << ",\n";
    json << "  \"velocity\": {\"x\": " << m_Velocity.x << ", \"y\": " << m_Velocity.y << ", \"z\": " << m_Velocity.z << "},\n";
    json << "  \"angularVelocity\": {\"x\": " << m_AngularVelocity.x << ", \"y\": " << m_AngularVelocity.y << ", \"z\": " << m_AngularVelocity.z << "},\n";
    json << "  \"freezePosition\": {\"x\": " << (m_FreezePositionX ? "true" : "false") << ", \"y\": " << (m_FreezePositionY ? "true" : "false") << ", \"z\": " << (m_FreezePositionZ ? "true" : "false") << "},\n";
    json << "  \"freezeRotation\": {\"x\": " << (m_FreezeRotationX ? "true" : "false") << ", \"y\": " << (m_FreezeRotationY ? "true" : "false") << ", \"z\": " << (m_FreezeRotationZ ? "true" : "false") << "}\n";
    json << "}";
    return json.str();
}

void Rigidbody::Deserialize(const std::string& json) {
    // Manual JSON parsing (simplified)
}

} // namespace LGE

