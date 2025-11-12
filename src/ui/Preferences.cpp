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

#include "LGE/ui/Preferences.h"
#include "imgui.h"

namespace LGE {

Preferences::Preferences()
    : m_Visible(false)
{
}

Preferences::~Preferences() {
}

void Preferences::OnUIRender() {
    if (!m_Visible) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Engine Preferences", &m_Visible)) {
        if (ImGui::BeginTabBar("PreferencesTabs")) {
            // General Settings
            if (ImGui::BeginTabItem("General")) {
                ImGui::Text("General Settings");
                ImGui::Separator();
                
                ImGui::Spacing();
                ImGui::Text("Editor");
                ImGui::Spacing();
                
                static bool autoSave = false;
                ImGui::Checkbox("Auto Save", &autoSave);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Automatically save the scene at regular intervals");
                }
                
                static int autoSaveInterval = 300;
                if (autoSave) {
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                    ImGui::InputInt("Interval (seconds)", &autoSaveInterval);
                    ImGui::PopItemWidth();
                }
                
                ImGui::Spacing();
                ImGui::Text("Performance");
                ImGui::Spacing();
                
                static bool showFPS = true;
                ImGui::Checkbox("Show FPS Counter", &showFPS);
                
                static int maxFPS = 60;
                ImGui::PushItemWidth(100.0f);
                ImGui::InputInt("Max FPS", &maxFPS);
                ImGui::PopItemWidth();
                
                ImGui::EndTabItem();
            }
            
            // Rendering Settings
            if (ImGui::BeginTabItem("Rendering")) {
                ImGui::Text("Rendering Settings");
                ImGui::Separator();
                
                ImGui::Spacing();
                ImGui::Text("Quality");
                ImGui::Spacing();
                
                static int renderQuality = 2;
                const char* qualityLevels[] = { "Low", "Medium", "High", "Ultra" };
                ImGui::Combo("Render Quality", &renderQuality, qualityLevels, IM_ARRAYSIZE(qualityLevels));
                
                ImGui::Spacing();
                ImGui::Text("Shadows");
                ImGui::Spacing();
                
                static bool enableShadows = true;
                ImGui::Checkbox("Enable Shadows", &enableShadows);
                
                static int shadowResolution = 2048;
                if (enableShadows) {
                    ImGui::PushItemWidth(150.0f);
                    ImGui::InputInt("Shadow Resolution", &shadowResolution);
                    ImGui::PopItemWidth();
                }
                
                ImGui::Spacing();
                ImGui::Text("Anti-Aliasing");
                ImGui::Spacing();
                
                static int antiAliasing = 1;
                const char* aaLevels[] = { "None", "FXAA", "MSAA 2x", "MSAA 4x", "MSAA 8x" };
                ImGui::Combo("Anti-Aliasing", &antiAliasing, aaLevels, IM_ARRAYSIZE(aaLevels));
                
                ImGui::EndTabItem();
            }
            
            // Input Settings
            if (ImGui::BeginTabItem("Input")) {
                ImGui::Text("Input Settings");
                ImGui::Separator();
                
                ImGui::Spacing();
                ImGui::Text("Mouse");
                ImGui::Spacing();
                
                static float mouseSensitivity = 0.5f;
                ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 0.1f, 2.0f);
                
                static bool invertMouseY = false;
                ImGui::Checkbox("Invert Mouse Y", &invertMouseY);
                
                ImGui::Spacing();
                ImGui::Text("Keyboard");
                ImGui::Spacing();
                
                static float movementSpeed = 5.0f;
                ImGui::SliderFloat("Movement Speed", &movementSpeed, 1.0f, 20.0f);
                
                ImGui::EndTabItem();
            }
            
            // Viewport Settings
            if (ImGui::BeginTabItem("Viewport")) {
                ImGui::Text("Viewport Settings");
                ImGui::Separator();
                
                ImGui::Spacing();
                ImGui::Text("Grid");
                ImGui::Spacing();
                
                static bool showGridByDefault = true;
                ImGui::Checkbox("Show Grid by Default", &showGridByDefault);
                
                static float gridSize = 1.0f;
                ImGui::PushItemWidth(100.0f);
                ImGui::InputFloat("Grid Size", &gridSize, 0.1f, 1.0f);
                ImGui::PopItemWidth();
                
                ImGui::Spacing();
                ImGui::Text("Gizmo");
                ImGui::Spacing();
                
                static float gizmoSize = 1.0f;
                ImGui::SliderFloat("Gizmo Size", &gizmoSize, 0.5f, 2.0f);
                
                static bool showGizmoLabels = true;
                ImGui::Checkbox("Show Gizmo Labels", &showGizmoLabels);
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Buttons at the bottom
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 200.0f);
        if (ImGui::Button("Reset to Defaults", ImVec2(100.0f, 0.0f))) {
            // Reset all settings to defaults
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply", ImVec2(80.0f, 0.0f))) {
            // Apply settings
        }
    }
    ImGui::End();
}

} // namespace LGE

