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

#include "LGE/core/components/LightComponent.h"
#include <algorithm>
#include <cstring>

namespace LGE {

LightComponent::LightComponent()
    : m_Type(ELightType::Directional)
    , m_Color(1.0f, 1.0f, 1.0f)
    , m_Intensity(1.0f)
    , m_Range(10.0f)
    , m_InnerCos(0.9f)  // ~25 degrees
    , m_OuterCos(0.7f)   // ~45 degrees
    , m_Width(1.0f)
    , m_Height(1.0f)
    , m_CastsShadows(false)
    , m_ShadowResolution(1024)
    , m_ShadowCascadeCount(4)
    , m_IESProfileId(-1)
{
    // Initialize cascade distances with defaults
    m_CascadeDistances[0] = 10.0f;
    m_CascadeDistances[1] = 25.0f;
    m_CascadeDistances[2] = 50.0f;
    m_CascadeDistances[3] = 100.0f;
}

void LightComponent::SetCascadeDistance(int index, float distance) {
    if (index >= 0 && index < 4) {
        m_CascadeDistances[index] = distance;
    }
}

float LightComponent::GetCascadeDistance(int index) const {
    if (index >= 0 && index < 4) {
        return m_CascadeDistances[index];
    }
    return 0.0f;
}

} // namespace LGE

