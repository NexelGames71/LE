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

#include "LGE/core/scene/components/SphereCollider.h"
#include <sstream>
#include <algorithm>

namespace LGE {

SphereCollider::SphereCollider()
    : m_Radius(0.5f)  // Default radius
{
}

void SphereCollider::SetRadius(float radius) {
    m_Radius = std::max(0.01f, radius);  // Minimum radius
}

std::string SphereCollider::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"SphereCollider\",\n";
    json << "  \"isTrigger\": " << (m_IsTrigger ? "true" : "false") << ",\n";
    json << "  \"offset\": {\"x\": " << m_Offset.x << ", \"y\": " << m_Offset.y << ", \"z\": " << m_Offset.z << "},\n";
    json << "  \"radius\": " << m_Radius << "\n";
    json << "}";
    return json.str();
}

void SphereCollider::Deserialize(const std::string& json) {
    // Manual JSON parsing (simplified)
    Collider::Deserialize(json);
}

} // namespace LGE

