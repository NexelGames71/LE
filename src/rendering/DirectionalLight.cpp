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

#include "LGE/rendering/DirectionalLight.h"

namespace LGE {

DirectionalLight::DirectionalLight()
    : m_Direction(0.0f, -1.0f, 0.0f)  // Pointing down by default
    , m_Color(1.0f, 1.0f, 1.0f)       // White light
    , m_Intensity(1.0f)
{
}

DirectionalLight::DirectionalLight(const Math::Vector3& direction, const Math::Vector3& color, float intensity)
    : m_Direction(direction)
    , m_Color(color)
    , m_Intensity(intensity)
{
}

} // namespace LGE

