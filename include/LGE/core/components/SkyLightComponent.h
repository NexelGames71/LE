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

#include "LGE/core/Component.h"
#include <cstdint>
#include <string>

namespace LGE {

// Forward declaration
class Texture;

// SkyLight source type
enum class ESkyLightSource : uint32_t {
    None = 0,
    HDR_Equirect = 1,
    Cubemap = 2,
    Procedural = 3
};

// SkyLight Component - Image-Based Lighting source
// Matches the spec: HDR equirect/cubemap, irradiance, prefilter, BRDF LUT
class SkyLightComponent : public Component {
public:
    SkyLightComponent();
    virtual ~SkyLightComponent() = default;

    // Component interface
    const char* GetTypeName() const override { return "SkyLightComponent"; }

    // Source type
    void SetSource(ESkyLightSource source) { m_Source = source; }
    ESkyLightSource GetSource() const { return m_Source; }

    // Environment HDR texture (original)
    void SetEnvironmentHDR(Texture* texture) { m_EnvHDR = texture; }
    Texture* GetEnvironmentHDR() const { return m_EnvHDR; }

    // Irradiance map (diffuse IBL, cube 32³)
    void SetIrradiance(Texture* texture) { m_Irradiance = texture; }
    Texture* GetIrradiance() const { return m_Irradiance; }

    // Prefilter map (specular IBL, cube 128–512², mipmapped)
    void SetPrefilter(Texture* texture) { m_Prefilter = texture; }
    Texture* GetPrefilter() const { return m_Prefilter; }

    // BRDF LUT (2D 512²)
    void SetBRDFLUT(Texture* texture) { m_BRDFLUT = texture; }
    Texture* GetBRDFLUT() const { return m_BRDFLUT; }

    // Baking state
    void SetRequiresBake(bool requires) { m_RequiresBake = requires; }
    bool GetRequiresBake() const { return m_RequiresBake; }

    // Path to HDR file (for loading)
    void SetHDRPath(const std::string& path) { m_HDRPath = path; }
    const std::string& GetHDRPath() const { return m_HDRPath; }

private:
    ESkyLightSource m_Source;
    Texture* m_EnvHDR;        // original HDR
    Texture* m_Irradiance;    // diffuse IBL (cube 32³)
    Texture* m_Prefilter;     // specular IBL (cube 128–512², mipmapped)
    Texture* m_BRDFLUT;       // 2D 512²
    bool m_RequiresBake;
    std::string m_HDRPath;    // Path to HDR file
};

} // namespace LGE

