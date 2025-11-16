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

#include "LGE/ui/UI.h"
#include "LGE/core/Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuizmo.h"

namespace LGE {

static bool s_Initialized = false;

void UI::Initialize(Window* window) {
    if (s_Initialized) {
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable docking
    
    // Set ini filename to persist layout (default is "imgui.ini" in current directory)
    // This ensures the layout is saved and restored between sessions
    io.IniFilename = "imgui.ini";
    
    // Configure modern font - larger size for better readability
    io.Fonts->Clear();
    
    // Try to load modern fonts (Windows)
    const char* fontPaths[] = {
        "C:/Windows/Fonts/segoeui.ttf",      // Segoe UI - modern Windows font
        "C:/Windows/Fonts/calibri.ttf",     // Calibri - clean modern font
        "C:/Windows/Fonts/arial.ttf",       // Arial - fallback
    };
    
    bool fontLoaded = false;
    for (const char* fontPath : fontPaths) {
        io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath, 14.5f);
        if (io.FontDefault) {
            fontLoaded = true;
            break;
        }
    }
    
    // Fallback: use default font but scale it up for modern look
    if (!fontLoaded) {
        io.FontDefault = io.Fonts->AddFontDefault();
        io.FontGlobalScale = 1.15f;  // Scale up default font for better readability
    }
    
    // Note: Font atlas building is handled automatically by the renderer backend
    // Do not call GetTexDataAsRGBA32 here - it will be called by ImGui_ImplOpenGL3_Init
    
    // Modern AAA Game Engine Theme
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    // Modern color palette - refined and professional
    colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);  // Slightly softer white
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);  // Slightly blue-tinted dark
    colors[ImGuiCol_ChildBg]                = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.15f, 0.15f, 0.16f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.25f, 0.25f, 0.28f, 0.60f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);  // Subtle shadow
    colors[ImGuiCol_FrameBg]                = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.30f, 0.30f, 0.32f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.40f, 0.40f, 0.42f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.50f, 0.50f, 0.52f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.40f, 0.70f, 1.00f, 1.00f);  // Modern blue accent
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.45f, 0.45f, 0.47f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.25f, 0.25f, 0.28f, 0.80f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.35f, 0.35f, 0.38f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.45f, 0.45f, 0.48f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.30f, 0.30f, 0.32f, 0.40f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.40f, 0.40f, 0.42f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.50f, 0.50f, 0.52f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.40f, 0.70f, 1.00f, 0.60f);  // Modern blue preview
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    
    // Modern styling - refined and polished
    style.WindowRounding = 0.0f;           // Sharp, modern windows
    style.ChildRounding = 0.0f;
    style.FrameRounding = 3.0f;             // Slightly more rounded for modern feel
    style.GrabRounding = 3.0f;
    style.PopupRounding = 4.0f;             // More rounded popups
    style.ScrollbarRounding = 3.0f;
    style.TabRounding = 3.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(10.0f, 10.0f);  // More spacious
    style.FramePadding = ImVec2(6.0f, 4.0f);     // Better button padding
    style.ItemSpacing = ImVec2(6.0f, 6.0f);       // More breathing room
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.IndentSpacing = 22.0f;
    style.ScrollbarSize = 16.0f;            // Slightly larger for better visibility
    style.GrabMinSize = 12.0f;
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window->GetNativeWindow());
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    s_Initialized = true;
}

void UI::Shutdown() {
    if (!s_Initialized) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    s_Initialized = false;
}

void UI::BeginFrame() {
    if (!s_Initialized) {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void UI::EndFrame() {
    if (!s_Initialized) {
        return;
    }

    ImGui::Render();
}

void UI::Render() {
    if (!s_Initialized) {
        return;
    }

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool UI::WantCaptureMouse() {
    if (!s_Initialized) {
        return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool UI::WantCaptureKeyboard() {
    if (!s_Initialized) {
        return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

} // namespace LGE


