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

#include "LGE/core/scene/components/PlayerController.h"
#include "LGE/core/scene/components/Collider.h"
#include "LGE/core/scene/GameObject.h"
#include <sstream>
#include <cmath>

namespace LGE {

PlayerController::PlayerController()
    : m_MoveSpeed(5.0f)
    , m_JumpForce(10.0f)
    , m_Rigidbody(nullptr)
    , m_Transform(nullptr)
{
}

void PlayerController::OnStart() {
    // Get required components
    if (m_Owner) {
        m_Rigidbody = m_Owner->GetComponent<Rigidbody>();
        m_Transform = m_Owner->GetTransform();
    }
}

void PlayerController::OnUpdate(float deltaTime) {
    if (!m_Transform) return;
    
    // Get input (placeholder - would use Input system)
    float horizontal = GetAxis("Horizontal");
    float vertical = GetAxis("Vertical");
    
    // Calculate movement direction
    Math::Vector3 movement(horizontal, 0.0f, vertical);
    
    // Normalize movement vector
    float length = std::sqrt(movement.x * movement.x + movement.y * movement.y + movement.z * movement.z);
    if (length > 0.0f) {
        movement.x /= length;
        movement.y /= length;
        movement.z /= length;
    }
    
    // Apply movement speed
    movement = movement * m_MoveSpeed * deltaTime;
    
    // Move using transform (or rigidbody if available)
    if (m_Rigidbody) {
        // Use physics-based movement
        m_Rigidbody->AddForce(movement, false);
    } else {
        // Use transform-based movement
        m_Transform->Translate(movement);
    }
    
    // Jump (placeholder - would check for key press)
    if (GetKeyDown(32) && m_Rigidbody) {  // Space key
        m_Rigidbody->AddForce(Math::Vector3(0.0f, m_JumpForce, 0.0f), true);
    }
}

void PlayerController::OnCollisionEnter(Collider* other) {
    if (!other || !other->GetGameObject()) return;
    
    // Example: Take damage when colliding with enemies
    // if (other->GetGameObject()->GetTag() == "Enemy") {
    //     // Take damage logic
    // }
}

float PlayerController::GetAxis(const std::string& axis) {
    // Placeholder - would query Input system
    // For now, return 0 (no input)
    return 0.0f;
}

bool PlayerController::GetKeyDown(int keyCode) {
    // Placeholder - would query Input system
    // For now, return false (no key pressed)
    return false;
}

std::string PlayerController::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"PlayerController\",\n";
    json << "  \"moveSpeed\": " << m_MoveSpeed << ",\n";
    json << "  \"jumpForce\": " << m_JumpForce << "\n";
    json << "}";
    return json.str();
}

void PlayerController::Deserialize(const std::string& json) {
    // Manual JSON parsing (simplified)
}

} // namespace LGE

