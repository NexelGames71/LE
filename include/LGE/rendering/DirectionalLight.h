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

class DirectionalLight {
public:
    DirectionalLight();
    DirectionalLight(const Math::Vector3& direction, const Math::Vector3& color, float intensity);

    void SetDirection(const Math::Vector3& direction) { m_Direction = direction; }
    void SetColor(const Math::Vector3& color) { m_Color = color; }
    void SetIntensity(float intensity) { m_Intensity = intensity; }

    const Math::Vector3& GetDirection() const { return m_Direction; }
    const Math::Vector3& GetColor() const { return m_Color; }
    float GetIntensity() const { return m_Intensity; }

private:
    Math::Vector3 m_Direction;  // Normalized direction (e.g., sun direction)
    Math::Vector3 m_Color;      // Light color (RGB)
    float m_Intensity;          // Light intensity
};

} // namespace LGE

