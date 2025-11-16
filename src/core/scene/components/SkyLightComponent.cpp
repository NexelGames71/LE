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

#include "LGE/core/scene/components/SkyLightComponent.h"
#include <sstream>

namespace LGE {

SkyLightComponent::SkyLightComponent() {
}

std::string SkyLightComponent::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"SkyLightComponent\",\n";
    json << "  \"enabled\": " << (Enabled ? "true" : "false") << ",\n";
    json << "  \"environmentMapPath\": \"" << EnvironmentMapPath << "\",\n";
    json << "  \"intensity\": " << Intensity << ",\n";
    json << "  \"useDiffuseIBL\": " << (UseDiffuseIBL ? "true" : "false") << ",\n";
    json << "  \"useSpecularIBL\": " << (UseSpecularIBL ? "true" : "false") << "\n";
    json << "}";
    return json.str();
}

void SkyLightComponent::Deserialize(const std::string& json) {
    // TODO: Implement JSON deserialization using nlohmann/json
}

} // namespace LGE

