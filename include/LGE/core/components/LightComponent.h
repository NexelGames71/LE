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
#include "LGE/math/Vector.h"
#include <cstdint>

namespace LGE {

// Light type enumeration (matches spec)
enum class ELightType : uint32_t {
    Directional = 0,
    Point = 1,
    Spot = 2,
    RectArea = 3,
    Sky = 4
};

// Light Component - Core light data structure
// Matches the spec: physical units, shadow cascades, IES profiles, etc.
class LightComponent : public Component {
public:
    LightComponent();
    virtual ~LightComponent() = default;

    // Component interface
    const char* GetTypeName() const override { return "LightComponent"; }

    // Type
    void SetType(ELightType type) { m_Type = type; }
    ELightType GetType() const { return m_Type; }

    // Color: linear RGB [0..inf]
    void SetColor(const Math::Vector3& color) { m_Color = color; }
    const Math::Vector3& GetColor() const { return m_Color; }

    // Intensity: lumens (point/spot/rect), lux (directional), candela (spot optional)
    void SetIntensity(float intensity) { m_Intensity = intensity; }
    float GetIntensity() const { return m_Intensity; }

    // Range: meters (for point/spot/rect)
    void SetRange(float range) { m_Range = range; }
    float GetRange() const { return m_Range; }

    // Spot light cone angles (cosine of angles)
    void SetInnerCone(float innerCos) { m_InnerCos = innerCos; }
    float GetInnerCone() const { return m_InnerCos; }
    void SetOuterCone(float outerCos) { m_OuterCos = outerCos; }
    float GetOuterCone() const { return m_OuterCos; }

    // Rect area light dimensions (meters)
    void SetWidth(float width) { m_Width = width; }
    float GetWidth() const { return m_Width; }
    void SetHeight(float height) { m_Height = height; }
    float GetHeight() const { return m_Height; }

    // Shadow settings
    void SetCastsShadows(bool castsShadows) { m_CastsShadows = castsShadows; }
    bool GetCastsShadows() const { return m_CastsShadows; }
    
    void SetShadowResolution(int resolution) { m_ShadowResolution = resolution; }
    int GetShadowResolution() const { return m_ShadowResolution; }

    // Directional light cascaded shadow maps
    void SetShadowCascadeCount(int count) { m_ShadowCascadeCount = count; }
    int GetShadowCascadeCount() const { return m_ShadowCascadeCount; }
    
    void SetCascadeDistance(int index, float distance);
    float GetCascadeDistance(int index) const;

    // IES profile ID (-1 if none)
    void SetIESProfileId(int id) { m_IESProfileId = id; }
    int GetIESProfileId() const { return m_IESProfileId; }

private:
    ELightType m_Type;
    Math::Vector3 m_Color;           // linear [0..inf]
    float m_Intensity;               // lm for point/spot/rect, lux for dir, cd for spot optional
    float m_Range;                  // meters
    float m_InnerCos, m_OuterCos;   // spot (cos of angles)
    float m_Width, m_Height;        // rect area (meters)
    bool m_CastsShadows;
    int m_ShadowResolution;          // per-light, optional
    int m_ShadowCascadeCount;       // dir only
    float m_CascadeDistances[4];    // max 4 cascades
    int m_IESProfileId;              // -1 if none
};

} // namespace LGE

