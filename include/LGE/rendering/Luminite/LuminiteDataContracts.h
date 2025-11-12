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
#include "LGE/math/Matrix.h"
#include <cstdint>

namespace LGE {
namespace Luminite {

// Light types
enum class LightType : uint32_t {
    Directional = 0,
    Point = 1,
    Spot = 2,
    Area = 3,
    Sky = 4
};

// Light flags
enum class LightFlags : uint32_t {
    None = 0,
    CastShadows = 1 << 0,
    Static = 1 << 1,
    Dynamic = 1 << 2,
    Baked = 1 << 3
};

// LightGPU - std140/StructuredBuffer compatible structure for GPU
// This structure is aligned for GPU buffer upload
struct LightGPU {
    // Position (for point/spot lights) or unused (for directional)
    Math::Vector3 position;
    float _padding0;
    
    // Direction (for directional/spot lights) or unused (for point)
    Math::Vector3 direction;
    float _padding1;
    
    // Color in linear space (RGB)
    Math::Vector3 color;
    float intensity;  // In lux (directional) or candela (point/spot)
    
    // Range and cone angles
    float range;       // For point/spot lights, 0 for directional
    float innerCone;   // For spot lights (cosine of inner angle)
    float outerCone;   // For spot lights (cosine of outer angle)
    float _padding2;
    
    // Type and flags
    uint32_t type;     // LightType enum value
    uint32_t flags;    // LightFlags bitmask
    float _padding3;
    float _padding4;
};

// FrameLightingUBO - Per-frame lighting uniform buffer
struct FrameLightingUBO {
    // View and projection matrices (for lighting calculations)
    Math::Matrix4 viewMatrix;
    Math::Matrix4 projectionMatrix;
    Math::Matrix4 viewProjectionMatrix;
    
    // Camera position
    Math::Vector3 cameraPosition;
    float exposure;  // Exposure value for tone mapping
    
    // Ambient lighting
    Math::Vector3 ambientColor;
    float ambientIntensity;
    
    // Feature flags (runtime toggles)
    uint32_t featureFlags;
    float _padding0;
    float _padding1;
    float _padding2;
    
    // Light counts
    uint32_t directionalLightCount;
    uint32_t pointLightCount;
    uint32_t spotLightCount;
    uint32_t totalLightCount;
    
    // Shadow settings
    float shadowBias;
    float shadowNormalBias;
    float shadowDistance;
    float _padding3;
};

} // namespace Luminite
} // namespace LGE

