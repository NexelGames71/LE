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

#include "LGE/rendering/Renderer.h"
#include "LGE/rendering/opengl/OpenGLRenderer.h"
#include "LGE/core/Log.h"

namespace LGE {

Renderer::API Renderer::s_API = Renderer::API::OpenGL;

std::unique_ptr<Renderer> Renderer::Create(API api, Window* window) {
    switch (api) {
        case API::OpenGL:
            Log::Info("Creating OpenGL renderer...");
            return std::make_unique<OpenGLRenderer>(window);
        
        case API::DirectX12:
            Log::Warn("DirectX12 renderer not yet implemented!");
            return nullptr;
        
        case API::Vulkan:
            Log::Warn("Vulkan renderer not yet implemented!");
            return nullptr;
        
        case API::None:
        default:
            Log::Error("No renderer API specified!");
            return nullptr;
    }
}

} // namespace LGE

