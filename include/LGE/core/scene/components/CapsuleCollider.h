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

#include "LGE/core/scene/components/Collider.h"
#include "LGE/core/scene/ComponentFactory.h"

namespace LGE {

// Capsule collider
class CapsuleCollider : public Collider {
public:
    CapsuleCollider();
    ~CapsuleCollider() override = default;
    
    // Component interface
    const char* GetTypeName() const override { return "CapsuleCollider"; }
    
    // Radius
    void SetRadius(float radius);
    float GetRadius() const { return m_Radius; }
    
    // Height
    void SetHeight(float height);
    float GetHeight() const { return m_Height; }
    
    // Direction (0=X, 1=Y, 2=Z)
    void SetDirection(int direction);
    int GetDirection() const { return m_Direction; }
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;

private:
    float m_Radius;
    float m_Height;
    int m_Direction;  // 0=X, 1=Y, 2=Z
};

REGISTER_COMPONENT(CapsuleCollider)

} // namespace LGE

