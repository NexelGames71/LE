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

namespace LGE {

enum class LightType {
    Directional,
    Point,
    Spot
};

// Light mobility (like Unreal Engine)
enum class LightMobility {
    Static,      // Pre-baked, cannot move or change at runtime
    Stationary,  // Can change intensity/color but not position/rotation
    Movable      // Fully dynamic, can move and change at runtime
};

// GPU-friendly light data structure (matches shader layout)
struct LightDataGPU {
    Math::Vector4 Position;      // w can be 1.0 for point, 0 for directional
    Math::Vector4 Direction;   // normalized, w unused
    Math::Vector4 ColorIntensity; // rgb = color, a = intensity
    float Range;                 // point/spot
    float InnerCone;             // radians
    float OuterCone;             // radians
    int Type;                    // LightType as int
    int CastShadows;             // bool, but as int for GPU
};

} // namespace LGE

