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

#include "LGE/core/scene/components/LightComponent.h"
#include <sstream>
#include <cmath>

namespace LGE {

LightComponent::LightComponent()
    : Type(LightType::Point)
    , Color(1.0f, 1.0f, 1.0f)
    , Intensity(10.0f)
    , Range(10.0f)
    , InnerAngle(0.349066f)  // ~20 degrees
    , OuterAngle(0.523599f)  // ~30 degrees
    , CastShadows(true)
    , ShadowBias(0.005f)
    , ShadowNormalBias(0.1f)
    , ShadowMapIndex(UINT32_MAX)
    , Mobility(LightMobility::Movable)
{
}

std::string LightComponent::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"LightComponent\",\n";
    json << "  \"lightType\": " << static_cast<int>(Type) << ",\n";
    json << "  \"color\": [" << Color.x << ", " << Color.y << ", " << Color.z << "],\n";
    json << "  \"intensity\": " << Intensity << ",\n";
    json << "  \"range\": " << Range << ",\n";
    json << "  \"innerAngle\": " << InnerAngle << ",\n";
    json << "  \"outerAngle\": " << OuterAngle << ",\n";
    json << "  \"castShadows\": " << (CastShadows ? "true" : "false") << ",\n";
    json << "  \"shadowBias\": " << ShadowBias << ",\n";
    json << "  \"shadowNormalBias\": " << ShadowNormalBias << ",\n";
    json << "  \"mobility\": " << static_cast<int>(Mobility) << "\n";
    json << "}";
    return json.str();
}

void LightComponent::Deserialize(const std::string& json) {
    // TODO: Implement JSON deserialization using nlohmann/json
}

} // namespace LGE
