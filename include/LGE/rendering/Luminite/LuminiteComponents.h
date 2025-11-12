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

#include "LGE/math/Vector.h"
#include "LGE/rendering/Luminite/LuminiteDataContracts.h"
#include <cstdint>
#include <string>

namespace LGE {
namespace Luminite {

// Base light component (common properties)
struct BaseLight {
    Math::Vector3 color = Math::Vector3(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    bool castShadows = false;
    bool enabled = true;
};

// DirectionalLight component (sun/moon)
struct DirectionalLight : public BaseLight {
    Math::Vector3 direction = Math::Vector3(0.0f, -1.0f, 0.0f);
};

// PointLight component (omni-directional)
struct PointLight : public BaseLight {
    float range = 10.0f;
    float falloffExponent = 2.0f;
};

// SpotLight component (cone-shaped)
struct SpotLight : public BaseLight {
    float range = 10.0f;
    float innerConeAngle = 30.0f;  // Degrees
    float outerConeAngle = 45.0f;  // Degrees
    Math::Vector3 direction = Math::Vector3(0.0f, -1.0f, 0.0f);
};

// SkyLight source type
enum class SkyLightSource {
    HDRCubemap,      // Use HDR cubemap texture
    CaptureScene     // Capture from scene (future)
};

// SkyLight component (IBL source - image-based lighting)
struct SkyLight {
    bool enabled = true;
    float intensity = 1.0f;
    
    // Source
    SkyLightSource source = SkyLightSource::HDRCubemap;
    std::string hdrTexturePath;  // Path to HDR equirectangular or cubemap
    
    // Generated IBL assets (cached paths)
    std::string irradianceMapPath;      // Cached irradiance cubemap
    std::string prefilterMapPath;       // Cached prefilter cubemap
    std::string brdfLUTPath;            // Cached BRDF LUT
    
    // Resolution settings
    uint32_t environmentMapResolution = 512;  // Source cubemap resolution
    uint32_t irradianceResolution = 32;      // Irradiance cubemap resolution
    uint32_t prefilterResolution = 128;      // Prefilter cubemap resolution
    uint32_t brdfLUTResolution = 512;         // BRDF LUT resolution
    
    // Runtime texture IDs (managed by subsystem)
    uint32_t environmentMapID = 0;
    uint32_t irradianceMapID = 0;
    uint32_t prefilterMapID = 0;
    uint32_t brdfLUTID = 0;
    
    // Baking state
    bool needsBaking = false;
    float bakeProgress = 0.0f;  // 0.0 to 1.0
};

// ReflectionProbe component
struct ReflectionProbe {
    bool enabled = true;
    Math::Vector3 position = Math::Vector3(0.0f, 0.0f, 0.0f);
    float influenceRadius = 10.0f;
    uint32_t resolution = 256;  // Cubemap resolution
    bool boxProjection = false;
    Math::Vector3 boxMin = Math::Vector3(-5.0f, -5.0f, -5.0f);
    Math::Vector3 boxMax = Math::Vector3(5.0f, 5.0f, 5.0f);
};

// Exposure mode
enum class ExposureMode {
    Manual,  // Manual exposure control
    Auto     // Automatic exposure based on scene luminance
};

// ExposureSettings component
struct ExposureSettings {
    // Mode
    ExposureMode mode = ExposureMode::Manual;
    
    // Manual exposure (EV100)
    float ev100 = 0.0f;  // Exposure value (EV100)
    
    // Camera settings for manual exposure
    float shutter = 1.0f / 60.0f;  // Shutter speed in seconds
    float iso = 100.0f;             // ISO sensitivity
    
    // Auto exposure settings
    float minLuma = 0.03f;   // Minimum luminance (dark scenes)
    float maxLuma = 1.0f;    // Maximum luminance (bright scenes)
    float targetLuma = 0.18f;  // Target luminance (middle gray)
    float autoExposureSpeed = 1.0f;  // How fast auto exposure adapts
    
    // Computed exposure value (updated by system)
    float exposure = 1.0f;
    
    // Legacy compatibility
    float minExposure = 0.1f;
    float maxExposure = 10.0f;
    bool autoExposure = false;  // Deprecated: use mode instead
};

// LuminiteEnvironment - Global lighting environment settings
struct LuminiteEnvironment {
    bool enabled = true;
    
    // Ambient lighting
    Math::Vector3 ambientColor = Math::Vector3(0.2f, 0.2f, 0.2f);
    float ambientIntensity = 0.3f;
    
    // Sky settings
    bool useSkyLight = true;
    float skyIntensity = 1.0f;
    
    // Exposure
    ExposureSettings exposure;
    
    // Feature toggles (runtime)
    uint32_t enabledFeatures = 0;  // FeatureFlag bitmask
    
    // Shadow settings
    float shadowDistance = 100.0f;
    float shadowBias = 0.005f;
    float shadowNormalBias = 0.02f;
};

} // namespace Luminite
} // namespace LGE

