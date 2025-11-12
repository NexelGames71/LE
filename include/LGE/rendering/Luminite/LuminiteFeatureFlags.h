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
namespace Luminite {

// Feature flags for Luminite lighting system
// These can be compile-time defines or runtime flags
enum class FeatureFlag : uint32_t {
    LUMA_LIT = 1 << 0,                    // Basic lighting enabled
    LUMA_IBL = 1 << 1,                    // Image-based lighting
    LUMA_SHADOWS = 1 << 2,                // Shadow mapping
    LUMA_CSM = 1 << 3,                    // Cascaded shadow maps
    LUMA_REFLECTION_PROBES = 1 << 4,      // Reflection probes
    LUMA_AREA_LIGHTS = 1 << 5,            // Area lights
    LUMA_VOLUMETRIC_FOG = 1 << 6,         // Volumetric fog/lighting
    LUMA_SSAO = 1 << 7,                   // Screen-space ambient occlusion
    LUMA_SSR = 1 << 8,                    // Screen-space reflections
    LUMA_TAA = 1 << 9,                    // Temporal anti-aliasing
    LUMA_BLOOM = 1 << 10,                 // Bloom post-processing
    LUMA_TONEMAPPING = 1 << 11,           // Tone mapping
};

// Helper functions for feature flags
inline uint32_t operator|(FeatureFlag a, FeatureFlag b) {
    return static_cast<uint32_t>(a) | static_cast<uint32_t>(b);
}

inline uint32_t operator|(uint32_t flags, FeatureFlag flag) {
    return flags | static_cast<uint32_t>(flag);
}

inline bool HasFeature(uint32_t flags, FeatureFlag flag) {
    return (flags & static_cast<uint32_t>(flag)) != 0;
}

} // namespace Luminite
} // namespace LGE

