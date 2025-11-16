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
#include <string>

namespace LGE {

class SkyLightComponent : public Component {
public:
    SkyLightComponent();
    ~SkyLightComponent() override = default;
    
    // Component interface
    const char* GetTypeName() const override { return "SkyLightComponent"; }
    bool IsUnique() const override { return true; } // Only one sky light per scene
    
    // Sky light properties
    bool Enabled = true;
    std::string EnvironmentMapPath; // HDR/EXR cubemap
    float Intensity = 1.0f;
    bool UseDiffuseIBL = true;
    bool UseSpecularIBL = true;
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;
};

REGISTER_COMPONENT(SkyLightComponent)

} // namespace LGE

