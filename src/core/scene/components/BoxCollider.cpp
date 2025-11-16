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

#include "LGE/core/scene/components/BoxCollider.h"
#include <sstream>

namespace LGE {

BoxCollider::BoxCollider()
    : m_Size(0.5f, 0.5f, 0.5f)  // Default 1x1x1 box (half-extents)
{
}

void BoxCollider::SetSize(const Math::Vector3& size) {
    m_Size = size;
    // Ensure positive values
    if (m_Size.x < 0.0f) m_Size.x = -m_Size.x;
    if (m_Size.y < 0.0f) m_Size.y = -m_Size.y;
    if (m_Size.z < 0.0f) m_Size.z = -m_Size.z;
}

std::string BoxCollider::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"BoxCollider\",\n";
    json << "  \"isTrigger\": " << (m_IsTrigger ? "true" : "false") << ",\n";
    json << "  \"offset\": {\"x\": " << m_Offset.x << ", \"y\": " << m_Offset.y << ", \"z\": " << m_Offset.z << "},\n";
    json << "  \"size\": {\"x\": " << m_Size.x << ", \"y\": " << m_Size.y << ", \"z\": " << m_Size.z << "}\n";
    json << "}";
    return json.str();
}

void BoxCollider::Deserialize(const std::string& json) {
    // Manual JSON parsing (simplified)
    Collider::Deserialize(json);
}

} // namespace LGE

