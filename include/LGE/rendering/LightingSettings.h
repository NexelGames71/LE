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

#include <cstdint>

namespace LGE {

// Forward declarations
class ProjectDescriptor;
class LightSystem;

// Global lighting settings for the project
struct LightingSettings {
    // Shadow settings
    bool EnableDynamicShadows = true;
    bool EnableCascadedShadows = false;  // Future: Cascaded Shadow Maps
    
    // Light limits
    int MaxDynamicLightsPerObject = 64;
    
    // SkyLight / IBL settings
    bool EnableSkyLightIBL = true;
    bool EnableDiffuseIBL = true;
    bool EnableSpecularIBL = true;
    
    // Future features (placeholders)
    bool EnableVolumetricFog = false;
    bool EnableSSAO = false;
    
    // Shadow map settings
    uint32_t ShadowMapSize = 2048;
    int MaxShadowCastingLights = 4;  // Limit how many lights can cast shadows simultaneously
    
    // Serialization helpers
    void LoadFromProjectDescriptor(ProjectDescriptor* descriptor);
    void SaveToProjectDescriptor(ProjectDescriptor* descriptor) const;
    
    // Apply settings to LightSystem
    void ApplyToLightSystem(LightSystem* lightSystem) const;
};

} // namespace LGE

