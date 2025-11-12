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

#include <memory>
#include <cstdint>

namespace LGE {

class Window;

// Abstract renderer interface
class Renderer {
public:
    enum class API {
        None = 0,
        OpenGL,
        DirectX12,
        Vulkan
    };

    virtual ~Renderer() = default;

    static std::unique_ptr<Renderer> Create(API api, Window* window);
    static API GetAPI() { return s_API; }
    static void SetAPI(API api) { s_API = api; }

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Clear(float r, float g, float b, float a) = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

protected:
    Window* m_Window;
    static API s_API;
};

} // namespace LGE

