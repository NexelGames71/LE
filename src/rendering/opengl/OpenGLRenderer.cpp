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

#include "LGE/rendering/opengl/OpenGLRenderer.h"
#include "LGE/core/Window.h"
#include "LGE/core/Log.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace LGE {

OpenGLRenderer::OpenGLRenderer(Window* window)
    : m_Initialized(false) {
    m_Window = window;
}

OpenGLRenderer::~OpenGLRenderer() {
    Shutdown();
}

void OpenGLRenderer::Init() {
    if (m_Initialized) {
        Log::Warn("OpenGL renderer already initialized!");
        return;
    }

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Log::Fatal("Failed to initialize GLAD!");
        return;
    }

    // Get OpenGL version info
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);

    Log::Info("OpenGL Renderer Initialized:");
    Log::Info("  Vendor: " + std::string(vendor));
    Log::Info("  Renderer: " + std::string(renderer));
    Log::Info("  Version: " + std::string(version));

    // Set initial viewport
    if (m_Window) {
        SetViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Enable face culling for better performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    m_Initialized = true;
    Log::Info("OpenGL renderer initialized successfully!");
}

void OpenGLRenderer::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    Log::Info("Shutting down OpenGL renderer...");
    m_Initialized = false;
}

void OpenGLRenderer::BeginFrame() {
    // Clear the screen
    Clear(0.1f, 0.1f, 0.1f, 1.0f);
}

void OpenGLRenderer::EndFrame() {
    // Frame end operations can be added here
}

void OpenGLRenderer::Clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    glViewport(static_cast<GLint>(x), static_cast<GLint>(y), 
               static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

} // namespace LGE

