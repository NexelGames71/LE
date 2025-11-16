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

#include "LGE/core/scene/Component.h"
#include "LGE/core/scene/GameObject.h"
#include "LGE/core/scene/components/Transform.h"
#include <sstream>

namespace LGE {

Component::Component()
    : m_GUID(GUID::Generate())
    , m_Owner(nullptr)
    , m_Enabled(true)
    , m_HasStarted(false)
{
}

void Component::SetEnabled(bool enabled) {
    if (m_Enabled == enabled) return;
    
    m_Enabled = enabled;
    
    if (enabled) {
        OnEnable();
    } else {
        OnDisable();
    }
}

Transform* Component::GetTransform() const {
    if (m_Owner) {
        return m_Owner->GetTransform();
    }
    return nullptr;
}

std::string Component::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"guid\": \"" << m_GUID.ToString() << "\",\n";
    json << "  \"type\": \"" << GetTypeName() << "\",\n";
    json << "  \"enabled\": " << (m_Enabled ? "true" : "false") << "\n";
    json << "}";
    return json.str();
}

void Component::Deserialize(const std::string& json) {
    // Manual JSON parsing (simplified)
    // In a full implementation, use proper JSON parsing
    // Extract GUID and enabled state
    size_t guidPos = json.find("\"guid\"");
    if (guidPos != std::string::npos) {
        size_t colonPos = json.find(':', guidPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                std::string guidStr = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                m_GUID = GUID::FromString(guidStr);
            }
        }
    }
    
    size_t enabledPos = json.find("\"enabled\"");
    if (enabledPos != std::string::npos) {
        size_t colonPos = json.find(':', enabledPos);
        size_t truePos = json.find("true", colonPos);
        size_t falsePos = json.find("false", colonPos);
        if (truePos != std::string::npos && (falsePos == std::string::npos || truePos < falsePos)) {
            m_Enabled = true;
        } else if (falsePos != std::string::npos) {
            m_Enabled = false;
        }
    }
}

} // namespace LGE


