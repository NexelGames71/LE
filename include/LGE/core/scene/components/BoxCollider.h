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
#include "LGE/math/Vector.h"

namespace LGE {

// Box collider - axis-aligned bounding box
class BoxCollider : public Collider {
public:
    BoxCollider();
    ~BoxCollider() override = default;
    
    // Component interface
    const char* GetTypeName() const override { return "BoxCollider"; }
    
    // Size (half-extents)
    void SetSize(const Math::Vector3& size);
    Math::Vector3 GetSize() const { return m_Size; }
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;

private:
    Math::Vector3 m_Size;  // Half-extents
};

REGISTER_COMPONENT(BoxCollider)

} // namespace LGE

