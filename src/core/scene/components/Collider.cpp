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

#include "LGE/core/scene/components/Collider.h"
#include <sstream>

namespace LGE {

Collider::Collider()
    : m_IsTrigger(false)
    , m_Offset(0.0f, 0.0f, 0.0f)
{
}

std::string Collider::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"Collider\",\n";
    json << "  \"isTrigger\": " << (m_IsTrigger ? "true" : "false") << ",\n";
    json << "  \"offset\": {\"x\": " << m_Offset.x << ", \"y\": " << m_Offset.y << ", \"z\": " << m_Offset.z << "}\n";
    json << "}";
    return json.str();
}

void Collider::Deserialize(const std::string& json) {
    // Manual JSON parsing (simplified)
}

} // namespace LGE

