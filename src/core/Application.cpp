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

#include "LGE/core/Application.h"
#include "LGE/core/Window.h"
#include "LGE/rendering/Renderer.h"
#include "LGE/core/Log.h"
#include "LGE/ui/UI.h"
#include <chrono>

namespace LGE {

Application::Application(const std::string& name)
    : m_Name(name), m_Running(false) {
}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    Log::Info("Initializing " + m_Name + "...");

    // Create window
    WindowProperties props;
    props.Title = m_Name;
    m_Window = std::make_unique<Window>(props);

    // Create renderer (OpenGL for now)
    Renderer::SetAPI(Renderer::API::OpenGL);
    m_Renderer = Renderer::Create(Renderer::GetAPI(), m_Window.get());
    
    if (!m_Renderer) {
        Log::Error("Failed to create renderer!");
        return false;
    }

    m_Renderer->Init();

    // Initialize UI system (after OpenGL is initialized)
    UI::Initialize(m_Window.get());

    m_Running = true;
    Log::Info("Application initialized successfully!");
    return true;
}

void Application::Run() {
    if (!m_Running) {
        Log::Error("Application not initialized!");
        return;
    }

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_Running && !m_Window->ShouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Begin UI frame
        UI::BeginFrame();
        
        m_Renderer->BeginFrame();
        
        OnUpdate(deltaTime);
        OnRender();
        
        m_Renderer->EndFrame();
        
        // End UI frame and render
        UI::EndFrame();
        UI::Render();
        
        m_Window->SwapBuffers();
        m_Window->OnUpdate();
    }
}

void Application::Shutdown() {
    // Shutdown UI system
    UI::Shutdown();

    if (m_Renderer) {
        m_Renderer->Shutdown();
        m_Renderer.reset();
    }

    if (m_Window) {
        m_Window.reset();
    }

    m_Running = false;
    Log::Info("Application shut down.");
}

} // namespace LGE

