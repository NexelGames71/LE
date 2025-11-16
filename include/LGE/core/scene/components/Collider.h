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

namespace LGE {

// Base Collider class - all colliders inherit from this
class Collider : public Component {
public:
    Collider();
    virtual ~Collider() = default;
    
    // Component interface
    const char* GetTypeName() const override { return "Collider"; }
    
    // Is trigger (doesn't cause physics collisions, only triggers events)
    void SetIsTrigger(bool isTrigger) { m_IsTrigger = isTrigger; }
    bool GetIsTrigger() const { return m_IsTrigger; }
    
    // Offset from transform
    void SetOffset(const Math::Vector3& offset) { m_Offset = offset; }
    Math::Vector3 GetOffset() const { return m_Offset; }
    
    // Serialization (base implementation)
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;

protected:
    bool m_IsTrigger;
    Math::Vector3 m_Offset;
};

} // namespace LGE

