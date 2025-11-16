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

#include "LGE/rendering/LightingSettings.h"
#include "LGE/core/project/ProjectDescriptor.h"
#include "LGE/rendering/LightSystem.h"
#include "LGE/core/Log.h"
#include <sstream>

namespace LGE {

void LightingSettings::LoadFromProjectDescriptor(ProjectDescriptor* descriptor) {
    if (!descriptor) return;
    
    // Load settings from project descriptor
    EnableDynamicShadows = descriptor->GetSetting("lighting.enableDynamicShadows", "true") == "true";
    EnableCascadedShadows = descriptor->GetSetting("lighting.enableCascadedShadows", "false") == "true";
    
    std::string maxLightsStr = descriptor->GetSetting("lighting.maxDynamicLightsPerObject", "64");
    try {
        MaxDynamicLightsPerObject = std::stoi(maxLightsStr);
    } catch (...) {
        MaxDynamicLightsPerObject = 64;
    }
    
    EnableSkyLightIBL = descriptor->GetSetting("lighting.enableSkyLightIBL", "true") == "true";
    EnableDiffuseIBL = descriptor->GetSetting("lighting.enableDiffuseIBL", "true") == "true";
    EnableSpecularIBL = descriptor->GetSetting("lighting.enableSpecularIBL", "true") == "true";
    
    EnableVolumetricFog = descriptor->GetSetting("lighting.enableVolumetricFog", "false") == "true";
    EnableSSAO = descriptor->GetSetting("lighting.enableSSAO", "false") == "true";
    
    std::string shadowMapSizeStr = descriptor->GetSetting("lighting.shadowMapSize", "2048");
    try {
        ShadowMapSize = static_cast<uint32_t>(std::stoul(shadowMapSizeStr));
    } catch (...) {
        ShadowMapSize = 2048;
    }
    
    std::string maxShadowLightsStr = descriptor->GetSetting("lighting.maxShadowCastingLights", "4");
    try {
        MaxShadowCastingLights = std::stoi(maxShadowLightsStr);
    } catch (...) {
        MaxShadowCastingLights = 4;
    }
}

void LightingSettings::SaveToProjectDescriptor(ProjectDescriptor* descriptor) const {
    if (!descriptor) return;
    
    // Save settings to project descriptor
    descriptor->SetSetting("lighting.enableDynamicShadows", EnableDynamicShadows ? "true" : "false");
    descriptor->SetSetting("lighting.enableCascadedShadows", EnableCascadedShadows ? "true" : "false");
    descriptor->SetSetting("lighting.maxDynamicLightsPerObject", std::to_string(MaxDynamicLightsPerObject));
    descriptor->SetSetting("lighting.enableSkyLightIBL", EnableSkyLightIBL ? "true" : "false");
    descriptor->SetSetting("lighting.enableDiffuseIBL", EnableDiffuseIBL ? "true" : "false");
    descriptor->SetSetting("lighting.enableSpecularIBL", EnableSpecularIBL ? "true" : "false");
    descriptor->SetSetting("lighting.enableVolumetricFog", EnableVolumetricFog ? "true" : "false");
    descriptor->SetSetting("lighting.enableSSAO", EnableSSAO ? "true" : "false");
    descriptor->SetSetting("lighting.shadowMapSize", std::to_string(ShadowMapSize));
    descriptor->SetSetting("lighting.maxShadowCastingLights", std::to_string(MaxShadowCastingLights));
}

void LightingSettings::ApplyToLightSystem(LightSystem* lightSystem) const {
    if (!lightSystem) return;
    
    // Apply settings to LightSystem
    // Note: LightSystem will need to be updated to support these settings
    // For now, we'll log that settings are being applied
    Log::Info("Applying lighting settings to LightSystem:");
    Log::Info("  Dynamic Shadows: " + std::string(EnableDynamicShadows ? "Enabled" : "Disabled"));
    Log::Info("  Cascaded Shadows: " + std::string(EnableCascadedShadows ? "Enabled" : "Disabled"));
    Log::Info("  Max Lights Per Object: " + std::to_string(MaxDynamicLightsPerObject));
    Log::Info("  SkyLight IBL: " + std::string(EnableSkyLightIBL ? "Enabled" : "Disabled"));
    Log::Info("  Shadow Map Size: " + std::to_string(ShadowMapSize));
    
    // TODO: Actually apply these settings to LightSystem when it supports them
    // For example:
    // lightSystem->SetMaxLights(MaxDynamicLightsPerObject);
    // lightSystem->SetShadowMapSize(ShadowMapSize);
    // lightSystem->SetShadowsEnabled(EnableDynamicShadows);
}

} // namespace LGE

