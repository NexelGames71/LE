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

#include "LGE/core/scene/Component.h"
#include "LGE/core/scene/ComponentFactory.h"
#include "LGE/rendering/Lighting.h"
#include "LGE/math/Vector.h"
#include <cstdint>

namespace LGE {

class LightComponent : public Component {
public:
    LightComponent();
    ~LightComponent() override = default;
    
    // Component interface
    const char* GetTypeName() const override { return "LightComponent"; }
    bool IsUnique() const override { return false; } // Can have multiple lights per GameObject
    
    // Light properties (public for easy access)
    LightType Type = LightType::Point;
    
    Math::Vector3 Color = Math::Vector3(1.0f);
    float Intensity = 10.0f;      // "brightness" in engine units
    
    // For Point/Spot
    float Range = 10.0f;
    
    // For Spot
    float InnerAngle = 0.349066f;  // ~20 degrees in radians
    float OuterAngle = 0.523599f;  // ~30 degrees in radians
    
    // Shadows
    bool CastShadows = true;
    float ShadowBias = 0.005f;
    float ShadowNormalBias = 0.1f;
    
    // Runtime handles (IDs to shadow maps, etc.)
    uint32_t ShadowMapIndex = UINT32_MAX;
    
    // Light mobility (for baking/static lights)
    LightMobility Mobility = LightMobility::Movable;
    
    // Helpers
    bool IsDirectional() const { return Type == LightType::Directional; }
    bool IsPoint() const { return Type == LightType::Point; }
    bool IsSpot() const { return Type == LightType::Spot; }
    
    bool IsStatic() const { return Mobility == LightMobility::Static; }
    bool IsStationary() const { return Mobility == LightMobility::Stationary; }
    bool IsMovable() const { return Mobility == LightMobility::Movable; }
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;
};

REGISTER_COMPONENT(LightComponent)

} // namespace LGE
