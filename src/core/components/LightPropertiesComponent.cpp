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

#include "LGE/core/components/LightPropertiesComponent.h"
#include <algorithm>
#include <cmath>

namespace LGE {

LightPropertiesComponent::LightPropertiesComponent()
    : m_Color(1.0f, 0.98f, 0.95f)  // Warm white default
    , m_Intensity(1.0f)
    , m_Temperature(5500.0f)  // Daylight default
    , m_IndirectIntensity(1.0f)
    , m_CastShadows(false)
    , m_VolumetricScattering(0.0f)
    , m_SpecularContribution(1.0f)
{
}

Math::Vector3 LightPropertiesComponent::GetColorFromTemperature() const {
    // Approximate blackbody radiation color from temperature
    // Based on the Planckian locus approximation
    
    float temp = m_Temperature / 100.0f;
    
    // Red component
    float red;
    if (temp <= 66.0f) {
        red = 255.0f;
    } else {
        red = temp - 60.0f;
        red = 329.698727446f * std::pow(red, -0.1332047592f);
        red = std::max(0.0f, std::min(255.0f, red));
    }
    
    // Green component
    float green;
    if (temp <= 66.0f) {
        green = temp;
        green = 99.4708025861f * std::log(green) - 161.1195681661f;
        green = std::max(0.0f, std::min(255.0f, green));
    } else {
        green = temp - 60.0f;
        green = 288.1221695283f * std::pow(green, -0.0755148492f);
        green = std::max(0.0f, std::min(255.0f, green));
    }
    
    // Blue component
    float blue;
    if (temp >= 66.0f) {
        blue = 255.0f;
    } else {
        if (temp <= 19.0f) {
            blue = 0.0f;
        } else {
            blue = temp - 10.0f;
            blue = 138.5177312231f * std::log(blue) - 305.0447927307f;
            blue = std::max(0.0f, std::min(255.0f, blue));
        }
    }
    
    // Convert from 0-255 range to 0-1 range (linear color space)
    return Math::Vector3(red / 255.0f, green / 255.0f, blue / 255.0f);
}

} // namespace LGE



