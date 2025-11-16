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

#include "LGE/core/scene/components/FogComponent.h"
#include <sstream>

namespace LGE {

FogComponent::FogComponent()
    : Enabled(true)
    , Type(FogType::Linear)
    , Color(0.5f, 0.6f, 0.7f)
    , StartDistance(10.0f)
    , EndDistance(50.0f)
    , Density(0.01f)
    , Height(0.0f)
    , HeightFalloff(1.0f)
    , EnableVolumetric(false)
    , VolumetricScattering(0.5f)
    , VolumetricExtinction(0.1f)
    , EnableLightShafts(false)
{
}

std::string FogComponent::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"FogComponent\",\n";
    json << "  \"enabled\": " << (Enabled ? "true" : "false") << ",\n";
    json << "  \"fogType\": " << static_cast<int>(Type) << ",\n";
    json << "  \"color\": {\"x\": " << Color.x << ", \"y\": " << Color.y << ", \"z\": " << Color.z << "},\n";
    json << "  \"startDistance\": " << StartDistance << ",\n";
    json << "  \"endDistance\": " << EndDistance << ",\n";
    json << "  \"density\": " << Density << ",\n";
    json << "  \"height\": " << Height << ",\n";
    json << "  \"heightFalloff\": " << HeightFalloff << ",\n";
    json << "  \"enableVolumetric\": " << (EnableVolumetric ? "true" : "false") << ",\n";
    json << "  \"volumetricScattering\": " << VolumetricScattering << ",\n";
    json << "  \"volumetricExtinction\": " << VolumetricExtinction << ",\n";
    json << "  \"enableLightShafts\": " << (EnableLightShafts ? "true" : "false") << "\n";
    json << "}";
    return json.str();
}

void FogComponent::Deserialize(const std::string& json) {
    // TODO: Implement JSON deserialization using nlohmann/json
}

} // namespace LGE

