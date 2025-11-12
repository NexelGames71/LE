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

#include "LGE/ui/ProjectSettings.h"
#include "imgui.h"

namespace LGE {

ProjectSettings::ProjectSettings()
    : m_Visible(false)
{
}

ProjectSettings::~ProjectSettings() {
}

void ProjectSettings::OnUIRender() {
    if (!m_Visible) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Project Settings", &m_Visible)) {
        // Two-column layout: Categories on left, content on right
        // Left panel for categories
        ImGui::BeginChild("Categories", ImVec2(200, 0), true, ImGuiWindowFlags_NoScrollbar);
        
        // Category list
        const char* categories[] = {
            "Build",
            "Input",
            "Graphics",
            "Rendering",
            "Audio",
            "Physics",
            "Networking",
            "Plugins"
        };
        
        static int selectedCategory = 2; // Default to Graphics
        
        ImGui::Spacing();
        for (int i = 0; i < IM_ARRAYSIZE(categories); i++) {
            bool isSelected = (selectedCategory == i);
            if (ImGui::Selectable(categories[i], isSelected, 0, ImVec2(0, 0))) {
                selectedCategory = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        
        ImGui::EndChild();
        
        // Vertical separator
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();
        
        // Content area
        ImGui::BeginChild("Content", ImVec2(0, 0), true);
        
        // Render content based on selected category
        if (selectedCategory == 0) { // Build
            ImGui::Text("Build Settings");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Build configuration options will be displayed here.");
        }
        else if (selectedCategory == 1) { // Input
            ImGui::Text("Input Settings");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Input configuration options will be displayed here.");
        }
        else if (selectedCategory == 2) { // Graphics
            ImGui::Text("Graphics Settings");
            ImGui::Separator();
            
            ImGui::Spacing();
            if (ImGui::CollapsingHeader("Luminite Lighting System", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Spacing();
                ImGui::Text("Feature Flags (Runtime Toggles)");
                ImGui::Spacing();
                
                // Runtime feature toggles
                static bool lumaLit = true;
                static bool lumaIBL = false;
                static bool lumaShadows = false;
                static bool lumaCSM = false;
                static bool lumaReflectionProbes = false;
                static bool lumaAreaLights = false;
                static bool lumaVolumetricFog = false;
                static bool lumaSSAO = false;
                static bool lumaSSR = false;
                static bool lumaTAA = false;
                static bool lumaBloom = false;
                static bool lumaToneMapping = false;
                
                ImGui::Checkbox("Basic Lighting (LUMA_LIT)", &lumaLit);
                ImGui::Checkbox("Image-Based Lighting (LUMA_IBL)", &lumaIBL);
                ImGui::Checkbox("Shadows (LUMA_SHADOWS)", &lumaShadows);
                ImGui::Checkbox("Cascaded Shadow Maps (LUMA_CSM)", &lumaCSM);
                ImGui::Checkbox("Reflection Probes (LUMA_REFLECTION_PROBES)", &lumaReflectionProbes);
                ImGui::Checkbox("Area Lights (LUMA_AREA_LIGHTS)", &lumaAreaLights);
                ImGui::Checkbox("Volumetric Fog (LUMA_VOLUMETRIC_FOG)", &lumaVolumetricFog);
                ImGui::Checkbox("SSAO (LUMA_SSAO)", &lumaSSAO);
                ImGui::Checkbox("Screen-Space Reflections (LUMA_SSR)", &lumaSSR);
                ImGui::Checkbox("Temporal AA (LUMA_TAA)", &lumaTAA);
                ImGui::Checkbox("Bloom (LUMA_BLOOM)", &lumaBloom);
                ImGui::Checkbox("Tone Mapping (LUMA_TONEMAPPING)", &lumaToneMapping);
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Text("Note: These are runtime toggles. Compile-time feature flags");
                ImGui::Text("can be set in the build configuration.");
            }
        }
        else if (selectedCategory == 3) { // Rendering
            ImGui::Text("Rendering Settings");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Rendering configuration options will be displayed here.");
        }
        else if (selectedCategory == 4) { // Audio
            ImGui::Text("Audio Settings");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Audio configuration options will be displayed here.");
        }
        else if (selectedCategory == 5) { // Physics
            ImGui::Text("Physics Settings");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Physics configuration options will be displayed here.");
        }
        else if (selectedCategory == 6) { // Networking
            ImGui::Text("Networking Settings");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Networking configuration options will be displayed here.");
        }
        else if (selectedCategory == 7) { // Plugins
            ImGui::Text("Plugins Settings");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Plugin configuration options will be displayed here.");
        }
        
        ImGui::EndChild();
    }
    ImGui::End();
}

} // namespace LGE

