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

#include "LGE/core/scene/components/CapsuleCollider.h"
#include <sstream>
#include <algorithm>

namespace LGE {

CapsuleCollider::CapsuleCollider()
    : m_Radius(0.5f)
    , m_Height(2.0f)
    , m_Direction(1)  // Default: Y-axis (vertical)
{
}

void CapsuleCollider::SetRadius(float radius) {
    m_Radius = std::max(0.01f, radius);
}

void CapsuleCollider::SetHeight(float height) {
    m_Height = std::max(0.01f, height);
}

void CapsuleCollider::SetDirection(int direction) {
    m_Direction = std::max(0, std::min(2, direction));  // Clamp to 0-2
}

std::string CapsuleCollider::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"CapsuleCollider\",\n";
    json << "  \"isTrigger\": " << (m_IsTrigger ? "true" : "false") << ",\n";
    json << "  \"offset\": {\"x\": " << m_Offset.x << ", \"y\": " << m_Offset.y << ", \"z\": " << m_Offset.z << "},\n";
    json << "  \"radius\": " << m_Radius << ",\n";
    json << "  \"height\": " << m_Height << ",\n";
    json << "  \"direction\": " << m_Direction << "\n";
    json << "}";
    return json.str();
}

void CapsuleCollider::Deserialize(const std::string& json) {
    // Manual JSON parsing (simplified)
    Collider::Deserialize(json);
}

} // namespace LGE

