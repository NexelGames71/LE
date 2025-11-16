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

namespace LGE {

// Rigidbody component - physics body
class Rigidbody : public Component {
public:
    Rigidbody();
    ~Rigidbody() override = default;
    
    // Component interface
    const char* GetTypeName() const override { return "Rigidbody"; }
    
    // Mass
    void SetMass(float mass);
    float GetMass() const { return m_Mass; }
    
    // Drag (linear damping)
    void SetDrag(float drag);
    float GetDrag() const { return m_Drag; }
    
    // Angular drag (angular damping)
    void SetAngularDrag(float angularDrag);
    float GetAngularDrag() const { return m_AngularDrag; }
    
    // Gravity
    void SetUseGravity(bool useGravity);
    bool GetUseGravity() const { return m_UseGravity; }
    
    // Velocity
    void SetVelocity(const Math::Vector3& velocity);
    Math::Vector3 GetVelocity() const { return m_Velocity; }
    
    // Angular velocity
    void SetAngularVelocity(const Math::Vector3& angularVelocity);
    Math::Vector3 GetAngularVelocity() const { return m_AngularVelocity; }
    
    // Forces
    void AddForce(const Math::Vector3& force, bool impulse = false);
    void AddTorque(const Math::Vector3& torque, bool impulse = false);
    
    // Constraints (freeze position/rotation on axes)
    void SetFreezePosition(bool x, bool y, bool z);
    void SetFreezeRotation(bool x, bool y, bool z);
    bool GetFreezePositionX() const { return m_FreezePositionX; }
    bool GetFreezePositionY() const { return m_FreezePositionY; }
    bool GetFreezePositionZ() const { return m_FreezePositionZ; }
    bool GetFreezeRotationX() const { return m_FreezeRotationX; }
    bool GetFreezeRotationY() const { return m_FreezeRotationY; }
    bool GetFreezeRotationZ() const { return m_FreezeRotationZ; }
    
    // Physics update (called by physics system)
    void PhysicsUpdate(float fixedDeltaTime);
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;

private:
    float m_Mass;
    float m_Drag;
    float m_AngularDrag;
    bool m_UseGravity;
    
    Math::Vector3 m_Velocity;
    Math::Vector3 m_AngularVelocity;
    Math::Vector3 m_AccumulatedForce;
    Math::Vector3 m_AccumulatedTorque;
    
    bool m_FreezePositionX, m_FreezePositionY, m_FreezePositionZ;
    bool m_FreezeRotationX, m_FreezeRotationY, m_FreezeRotationZ;
    
    static const Math::Vector3 s_Gravity;
};

REGISTER_COMPONENT(Rigidbody)

} // namespace LGE

