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

#include "LGE/core/scene/components/ScriptComponent.h"
#include "LGE/core/scene/components/Rigidbody.h"
#include "LGE/core/scene/components/Transform.h"
#include "LGE/math/Vector.h"
#include "LGE/core/scene/ComponentFactory.h"

namespace LGE {

// Example user script - PlayerController
// Demonstrates how to create custom behavior scripts
class PlayerController : public ScriptComponent {
public:
    PlayerController();
    ~PlayerController() override = default;
    
    // Component interface
    const char* GetTypeName() const override { return "PlayerController"; }
    
    // Movement settings
    void SetMoveSpeed(float speed) { m_MoveSpeed = speed; }
    float GetMoveSpeed() const { return m_MoveSpeed; }
    
    void SetJumpForce(float force) { m_JumpForce = force; }
    float GetJumpForce() const { return m_JumpForce; }
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;

protected:
    void OnStart() override;
    void OnUpdate(float deltaTime) override;
    void OnCollisionEnter(Collider* other) override;

private:
    float m_MoveSpeed;
    float m_JumpForce;
    Rigidbody* m_Rigidbody;
    Transform* m_Transform;
    
    // Helper methods (would use Input system when available)
    float GetAxis(const std::string& axis);
    bool GetKeyDown(int keyCode);
};

} // namespace LGE

REGISTER_COMPONENT(PlayerController)

