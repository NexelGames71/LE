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

#include "LGE/rendering/Renderer.h"

namespace LGE {

class OpenGLRenderer : public Renderer {
public:
    OpenGLRenderer(Window* window);
    virtual ~OpenGLRenderer();

    void Init() override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    void Clear(float r, float g, float b, float a) override;
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

private:
    bool m_Initialized;
};

} // namespace LGE

