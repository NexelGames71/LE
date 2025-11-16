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
#include "LGE/math/Vector.h"

namespace LGE {

// Light Properties Component
// Holds the main light data that the shader or lighting system uses
class LightPropertiesComponent : public Component {
public:
    LightPropertiesComponent();
    virtual ~LightPropertiesComponent() = default;

    // Component interface
    const char* GetTypeName() const override { return "LightPropertiesComponent"; }

    // Color: RGB value (linear color space)
    void SetColor(const Math::Vector3& color) { m_Color = color; }
    const Math::Vector3& GetColor() const { return m_Color; }

    // Intensity: Brightness in lux or arbitrary units
    void SetIntensity(float intensity) { m_Intensity = intensity; }
    float GetIntensity() const { return m_Intensity; }

    // Temperature: Color temperature (e.g., 5500K daylight, 3000K warm light)
    void SetTemperature(float temperature) { m_Temperature = temperature; }
    float GetTemperature() const { return m_Temperature; }

    // IndirectIntensity: Scale factor for how much light it contributes to global illumination
    void SetIndirectIntensity(float indirectIntensity) { m_IndirectIntensity = indirectIntensity; }
    float GetIndirectIntensity() const { return m_IndirectIntensity; }

    // CastShadows: Boolean — toggles shadow rendering
    void SetCastShadows(bool castShadows) { m_CastShadows = castShadows; }
    bool GetCastShadows() const { return m_CastShadows; }

    // VolumetricScattering: Strength of light in fog or volumetric effects
    void SetVolumetricScattering(float volumetricScattering) { m_VolumetricScattering = volumetricScattering; }
    float GetVolumetricScattering() const { return m_VolumetricScattering; }

    // SpecularContribution: Multiplier for specular highlights from this light
    void SetSpecularContribution(float specularContribution) { m_SpecularContribution = specularContribution; }
    float GetSpecularContribution() const { return m_SpecularContribution; }

    // Helper: Convert temperature to RGB color (approximate blackbody radiation)
    Math::Vector3 GetColorFromTemperature() const;

private:
    Math::Vector3 m_Color;           // RGB color (linear space)
    float m_Intensity;               // Brightness
    float m_Temperature;             // Color temperature in Kelvin
    float m_IndirectIntensity;       // GI contribution scale
    bool m_CastShadows;              // Shadow casting enabled
    float m_VolumetricScattering;    // Volumetric scattering strength
    float m_SpecularContribution;    // Specular highlight multiplier
};

} // namespace LGE



