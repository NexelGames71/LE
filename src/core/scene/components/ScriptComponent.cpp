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

#include "LGE/core/scene/components/ScriptComponent.h"
#include <sstream>

namespace LGE {

ScriptComponent::ScriptComponent() {
}

std::string ScriptComponent::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"ScriptComponent\"\n";
    json << "}";
    return json.str();
}

void ScriptComponent::Deserialize(const std::string& json) {
    // Manual JSON parsing (simplified)
}

} // namespace LGE

