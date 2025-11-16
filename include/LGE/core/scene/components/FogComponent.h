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
#include "LGE/math/Vector.h"
#include <string>

namespace LGE {

// Fog types
enum class FogType {
    None,
    Linear,
    Exponential,
    ExponentialSquared,
    Volumetric  // Future: Volumetric fog
};

class FogComponent : public Component {
public:
    FogComponent();
    ~FogComponent() override = default;
    
    // Component interface
    const char* GetTypeName() const override { return "FogComponent"; }
    bool IsUnique() const override { return true; } // Only one fog per scene
    
    // Fog properties
    bool Enabled = true;
    FogType Type = FogType::Linear;
    
    // Color
    Math::Vector3 Color = Math::Vector3(0.5f, 0.6f, 0.7f); // Sky blue
    
    // Linear fog
    float StartDistance = 10.0f;
    float EndDistance = 50.0f;
    
    // Exponential fog
    float Density = 0.01f;
    
    // Height fog (future)
    float Height = 0.0f;
    float HeightFalloff = 1.0f;
    
    // Volumetric fog (future)
    bool EnableVolumetric = false;
    float VolumetricScattering = 0.5f;
    float VolumetricExtinction = 0.1f;
    
    // Light shafts (god rays) - future
    bool EnableLightShafts = false;
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;
};

REGISTER_COMPONENT(FogComponent)

} // namespace LGE

