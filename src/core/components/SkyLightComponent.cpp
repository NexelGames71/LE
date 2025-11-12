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

#include "LGE/core/components/SkyLightComponent.h"

namespace LGE {

SkyLightComponent::SkyLightComponent()
    : m_Source(ESkyLightSource::None)
    , m_EnvHDR(nullptr)
    , m_Irradiance(nullptr)
    , m_Prefilter(nullptr)
    , m_BRDFLUT(nullptr)
    , m_RequiresBake(false)
{
}

} // namespace LGE

