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

#include "LGE/core/SplashScreen.h"
#include "LGE/core/Application.h"
#include "LGE/core/Window.h"
#include "LGE/core/LayerStack.h"
#include "LGE/core/Log.h"
#include "LGE/core/Input.h"
#include "LGE/rendering/Texture.h"
#include "LGE/ui/UI.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <memory>

namespace LGE {

SplashScreen::SplashScreen(const std::string& imagePath, float displayDuration)
    : Layer("SplashScreen")
    , m_ImagePath(imagePath)
    , m_State(State::FadingIn)
    , m_ElapsedTime(0.0f)
    , m_FadeInDuration(0.5f)
    , m_FadeOutDuration(0.5f)
    , m_DisplayDuration(displayDuration)
    , m_CurrentAlpha(0.0f)
    , m_LayerStackPtr(nullptr)
    , m_Window(nullptr)
    , m_OriginalWidth(0)
    , m_OriginalHeight(0)
    , m_WindowResized(false)
    , m_WindowScale(0.35f)  // Default to 35% of image size
{
}

void SplashScreen::OnAttach() {
    Log::Info("Attaching SplashScreen layer");
    
    // Load splash texture
    TextureSpec spec;
    spec.filepath = m_ImagePath;
    spec.gammaCorrected = true;
    spec.minFilter = TextureFilter::Linear;
    spec.magFilter = TextureFilter::Linear;
    spec.wrapS = TextureWrap::ClampToEdge;
    spec.wrapT = TextureWrap::ClampToEdge;
    spec.generateMipmaps = false;
    
    // Create texture directly (TextureManager is not static)
    auto texture = std::make_shared<Texture>();
    if (!texture->Load(spec)) {
        Log::Error("Failed to load splash screen image: " + m_ImagePath);
        m_State = State::Finished;
        return;
    }
    
    m_SplashTexture = texture;
    m_StartTime = std::chrono::high_resolution_clock::now();
    m_CurrentAlpha = 0.0f;
    m_State = State::FadingIn;
    
    // Resize window to match splash image size exactly
    if (m_Window) {
        m_OriginalWidth = m_Window->GetWidth();
        m_OriginalHeight = m_Window->GetHeight();
        
        int imageWidth = texture->GetWidth();
        int imageHeight = texture->GetHeight();
        
        if (imageWidth > 0 && imageHeight > 0) {
            // Calculate scaled dimensions maintaining aspect ratio
            float aspectRatio = static_cast<float>(imageWidth) / static_cast<float>(imageHeight);
            
            // Use scale for width, then calculate height to maintain aspect ratio
            uint32_t scaledWidth = static_cast<uint32_t>(imageWidth * m_WindowScale);
            // Reduce height slightly (about 5% less)
            uint32_t scaledHeight = static_cast<uint32_t>((scaledWidth / aspectRatio) * 0.95f);
            
            m_Window->SetSize(scaledWidth, scaledHeight);
            m_Window->CenterOnScreen();
            m_WindowResized = true;
            Log::Info("Window resized to splash image size (scaled " + std::to_string(m_WindowScale * 100.0f) + "%): " + 
                      std::to_string(scaledWidth) + "x" + std::to_string(scaledHeight) + " and centered");
        }
    }
    
    Log::Info("Splash screen loaded successfully");
}

void SplashScreen::OnDetach() {
    Log::Info("Detaching SplashScreen layer");
    
    // Restore original window size
    if (m_Window && m_WindowResized) {
        m_Window->SetSize(m_OriginalWidth, m_OriginalHeight);
        Log::Info("Window restored to original size: " + std::to_string(m_OriginalWidth) + "x" + std::to_string(m_OriginalHeight));
        m_WindowResized = false;
    }
    
    m_SplashTexture.reset();
}

void SplashScreen::OnUpdate(float deltaTime) {
    if (m_State == State::Finished) {
        return;
    }
    
    m_ElapsedTime += deltaTime;
    UpdateAlpha(deltaTime);
    
    // Check for skip input (any key or mouse button)
    // Check common keys (Space, Enter, Escape, or any mouse button)
    if (Input::IsKeyPressed(GLFW_KEY_SPACE) || 
        Input::IsKeyPressed(GLFW_KEY_ENTER) ||
        Input::IsKeyPressed(GLFW_KEY_ESCAPE) ||
        Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) ||
        Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) ||
        Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        Skip();
    }
}

void SplashScreen::OnRender() {
    if (m_State == State::Finished || !m_SplashTexture) {
        return;
    }
    
    // Set ImGui to fullscreen overlay
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoDocking;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    
    float alpha = GetCurrentAlpha();
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    
    // Force this window to be on top
    ImGui::SetNextWindowFocus();
    
    if (ImGui::Begin("SplashScreen", nullptr, flags)) {
        RenderFullscreenImage();
    }
    ImGui::End();
    
    ImGui::PopStyleVar(3);
}

void SplashScreen::OnEvent(Event& event) {
    // Skip on any input event
    event.SetHandled(true);
}

void SplashScreen::UpdateAlpha(float deltaTime) {
    switch (m_State) {
        case State::FadingIn:
            if (m_ElapsedTime >= m_FadeInDuration) {
                m_CurrentAlpha = 1.0f;
                m_State = State::Displaying;
                m_ElapsedTime = 0.0f;
            } else {
                m_CurrentAlpha = std::min(1.0f, m_ElapsedTime / m_FadeInDuration);
            }
            break;
            
        case State::Displaying:
            if (m_ElapsedTime >= m_DisplayDuration) {
                m_State = State::FadingOut;
                m_ElapsedTime = 0.0f;
            }
            break;
            
        case State::FadingOut:
            if (m_ElapsedTime >= m_FadeOutDuration) {
                m_CurrentAlpha = 0.0f;
                m_State = State::Finished;
                
                // Restore window size before removing from stack
                if (m_Window && m_WindowResized) {
                    m_Window->SetSize(m_OriginalWidth, m_OriginalHeight);
                    Log::Info("Window restored to original size: " + std::to_string(m_OriginalWidth) + "x" + std::to_string(m_OriginalHeight));
                    m_WindowResized = false;
                }
                
                // Remove ourselves from the layer stack
                if (m_LayerStackPtr) {
                    m_LayerStackPtr->RemoveOverlay(GetName());
                }
            } else {
                m_CurrentAlpha = 1.0f - std::min(1.0f, m_ElapsedTime / m_FadeOutDuration);
            }
            break;
            
        case State::Finished:
            break;
    }
}

float SplashScreen::GetCurrentAlpha() const {
    return m_CurrentAlpha;
}

void SplashScreen::RenderFullscreenImage() {
    if (!m_SplashTexture || m_SplashTexture->GetRendererID() == 0) {
        return;
    }
    
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    // Calculate aspect ratio and sizing
    int texWidth = m_SplashTexture->GetWidth();
    int texHeight = m_SplashTexture->GetHeight();
    
    if (texWidth == 0 || texHeight == 0) {
        return;
    }
    
    float texAspect = static_cast<float>(texWidth) / static_cast<float>(texHeight);
    float viewportAspect = viewportSize.x / viewportSize.y;
    
    ImVec2 imageSize;
    ImVec2 imagePos;
    
    // Fit image to viewport while maintaining aspect ratio
    if (texAspect > viewportAspect) {
        // Image is wider - fit to width
        imageSize.x = viewportSize.x;
        imageSize.y = viewportSize.x / texAspect;
        imagePos.x = 0.0f;
        imagePos.y = (viewportSize.y - imageSize.y) * 0.5f;
    } else {
        // Image is taller - fit to height
        imageSize.y = viewportSize.y;
        imageSize.x = viewportSize.y * texAspect;
        imagePos.x = (viewportSize.x - imageSize.x) * 0.5f;
        imagePos.y = 0.0f;
    }
    
    ImGui::SetCursorPos(imagePos);
    
    // Draw image with proper UV coordinates
    // ImGui uses top-left origin, so we use (0,0) to (1,1) for normal orientation
    ImGui::Image(
        reinterpret_cast<void*>(static_cast<intptr_t>(m_SplashTexture->GetRendererID())),
        imageSize,
        ImVec2(0, 0),  // UV min (top-left)
        ImVec2(1, 1)   // UV max (bottom-right)
    );
}

void SplashScreen::Skip() {
    if (m_State != State::Finished) {
        Log::Info("Splash screen skipped by user");
        m_State = State::Finished;
        m_CurrentAlpha = 0.0f;
        
        // Restore window size before removing from stack
        if (m_Window && m_WindowResized) {
            m_Window->SetSize(m_OriginalWidth, m_OriginalHeight);
            Log::Info("Window restored to original size: " + std::to_string(m_OriginalWidth) + "x" + std::to_string(m_OriginalHeight));
            m_WindowResized = false;
        }
        
        // Remove ourselves from the layer stack
        if (m_LayerStackPtr) {
            m_LayerStackPtr->RemoveOverlay(GetName());
        }
    }
}

} // namespace LGE

