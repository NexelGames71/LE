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
#include "LGE/core/project/Project.h"
#include "LGE/core/project/ProjectDescriptor.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/rendering/LightingSettings.h"
#include "LGE/core/scene/GameObject.h"
#include "LGE/core/scene/World.h"
#include "LGE/core/scene/components/SkyLightComponent.h"
#include "LGE/core/scene/SceneManager.h"
#include "imgui.h"
#include <vector>
#include <algorithm>
#include <sstream>

namespace LGE {

ProjectSettings::ProjectSettings()
    : m_Visible(false)
    , m_Project(nullptr)
    , m_LightingSettingsDirty(false)
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
            
            if (m_Project && m_Project->IsLoaded()) {
                // Default Scene setting
                ImGui::Text("Default Scene");
                ImGui::Spacing();
                
                // Get available scenes from the project
                std::string scenesPath = m_Project->GetScenesPath();
                std::vector<std::string> sceneFiles;
                
                // Scan for .lscene files
                if (LGE::FileSystem::Exists(scenesPath)) {
                    auto files = LGE::FileSystem::ListDirectory(scenesPath);
                    for (const auto& entry : files) {
                        if (!entry.isDirectory && entry.extension == ".lscene") {
                            sceneFiles.push_back(entry.name);
                        }
                    }
                }
                
                // Get current default scene
                std::string currentDefault = m_Project->GetDefaultScene();
                if (m_DefaultSceneBuffer.empty() || m_DefaultSceneBuffer != currentDefault) {
                    m_DefaultSceneBuffer = currentDefault;
                }
                
                // Combo box for scene selection
                ImGui::Text("Main Scene:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(300.0f);
                
                if (ImGui::BeginCombo("##DefaultScene", m_DefaultSceneBuffer.c_str())) {
                    for (const auto& scene : sceneFiles) {
                        bool isSelected = (m_DefaultSceneBuffer == scene);
                        if (ImGui::Selectable(scene.c_str(), isSelected)) {
                            m_DefaultSceneBuffer = scene;
                            m_Project->SetDefaultScene(scene);
                            m_Project->Save();
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
                
                ImGui::Spacing();
                ImGui::TextWrapped("The default scene will be loaded when the project starts.");
            } else {
                ImGui::Text("No project loaded.");
            }
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
        }
        else if (selectedCategory == 3) { // Rendering
            RenderLightingSettings();
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
    
    // Apply settings if they were changed
    if (m_LightingSettingsDirty && m_Project && m_Project->IsLoaded()) {
        SaveLightingSettings();
        if (m_LightSystem) {
            m_LightingSettings.ApplyToLightSystem(m_LightSystem);
        }
        m_LightingSettingsDirty = false;
    }
}

void ProjectSettings::LoadLightingSettings() {
    if (!m_Project || !m_Project->IsLoaded()) return;
    
    // Get ProjectDescriptor from Project
    // Note: Project doesn't expose GetDescriptor() directly, so we'll use a workaround
    // For now, we'll load from the project file path
    std::string projectFilePath = m_Project->GetProjectFilePath();
    if (!projectFilePath.empty()) {
        ProjectDescriptor descriptor;
        if (descriptor.Load(projectFilePath)) {
            m_LightingSettings.LoadFromProjectDescriptor(&descriptor);
        }
    }
}

void ProjectSettings::SaveLightingSettings() {
    if (!m_Project || !m_Project->IsLoaded()) return;
    
    // Save to project file
    std::string projectFilePath = m_Project->GetProjectFilePath();
    if (!projectFilePath.empty()) {
        ProjectDescriptor descriptor;
        if (descriptor.Load(projectFilePath)) {
            m_LightingSettings.SaveToProjectDescriptor(&descriptor);
            descriptor.Save(projectFilePath);
        }
    }
}

void ProjectSettings::RenderLightingSettings() {
    ImGui::Text("Lighting Settings");
    ImGui::Separator();
    ImGui::Spacing();
    
    if (!m_Project || !m_Project->IsLoaded()) {
        ImGui::Text("No project loaded.");
        return;
    }
    
    // Load settings on first render
    static bool settingsLoaded = false;
    if (!settingsLoaded) {
        LoadLightingSettings();
        settingsLoaded = true;
    }
    
    ImGui::PushID("LightingSettings");
    
    // Shadow Settings
    if (ImGui::CollapsingHeader("Shadows", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool enableShadows = m_LightingSettings.EnableDynamicShadows;
        if (ImGui::Checkbox("Enable Dynamic Shadows", &enableShadows)) {
            m_LightingSettings.EnableDynamicShadows = enableShadows;
            m_LightingSettingsDirty = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Enable shadow mapping for lights that cast shadows.");
            ImGui::EndTooltip();
        }
        
        if (enableShadows) {
            ImGui::Indent();
            
            bool enableCascaded = m_LightingSettings.EnableCascadedShadows;
            if (ImGui::Checkbox("Enable Cascaded Shadows", &enableCascaded)) {
                m_LightingSettings.EnableCascadedShadows = enableCascaded;
                m_LightingSettingsDirty = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Use Cascaded Shadow Maps (CSM) for better quality at multiple distances. (Future feature)");
                ImGui::EndTooltip();
            }
            
            int maxShadowLights = m_LightingSettings.MaxShadowCastingLights;
            if (ImGui::DragInt("Max Shadow Casting Lights", &maxShadowLights, 1.0f, 1, 16)) {
                m_LightingSettings.MaxShadowCastingLights = maxShadowLights;
                m_LightingSettingsDirty = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Maximum number of lights that can cast shadows simultaneously.");
                ImGui::EndTooltip();
            }
            
            uint32_t shadowMapSize = m_LightingSettings.ShadowMapSize;
            const char* sizes[] = { "512", "1024", "2048", "4096" };
            int currentSizeIndex = 0;
            if (shadowMapSize == 512) currentSizeIndex = 0;
            else if (shadowMapSize == 1024) currentSizeIndex = 1;
            else if (shadowMapSize == 2048) currentSizeIndex = 2;
            else if (shadowMapSize == 4096) currentSizeIndex = 3;
            
            if (ImGui::Combo("Shadow Map Size", &currentSizeIndex, sizes, 4)) {
                uint32_t newSize = 512;
                if (currentSizeIndex == 0) newSize = 512;
                else if (currentSizeIndex == 1) newSize = 1024;
                else if (currentSizeIndex == 2) newSize = 2048;
                else if (currentSizeIndex == 3) newSize = 4096;
                m_LightingSettings.ShadowMapSize = newSize;
                m_LightingSettingsDirty = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Resolution of shadow maps. Higher = better quality but more GPU memory.");
                ImGui::EndTooltip();
            }
            
            ImGui::Unindent();
        }
    }
    
    ImGui::Spacing();
    
    // Light Limits
    if (ImGui::CollapsingHeader("Light Limits", ImGuiTreeNodeFlags_DefaultOpen)) {
        int maxLights = m_LightingSettings.MaxDynamicLightsPerObject;
        if (ImGui::DragInt("Max Dynamic Lights Per Object", &maxLights, 1.0f, 1, 256)) {
            m_LightingSettings.MaxDynamicLightsPerObject = maxLights;
            m_LightingSettingsDirty = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Maximum number of lights that can affect a single object.");
            ImGui::EndTooltip();
        }
    }
    
    ImGui::Spacing();
    
    // SkyLight / IBL Settings
    if (ImGui::CollapsingHeader("SkyLight & Image-Based Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool enableIBL = m_LightingSettings.EnableSkyLightIBL;
        if (ImGui::Checkbox("Enable SkyLight IBL", &enableIBL)) {
            m_LightingSettings.EnableSkyLightIBL = enableIBL;
            m_LightingSettingsDirty = true;
            
            // Auto-add/remove SkyLightComponent to environment entity
            if (m_SceneManager) {
                auto activeWorld = m_SceneManager->GetActiveWorld();
                if (activeWorld) {
                    // Find or create environment entity
                    auto allObjects = activeWorld->GetAllGameObjects();
                    std::shared_ptr<GameObject> envEntity = nullptr;
                    
                    for (const auto& obj : allObjects) {
                        if (obj && obj->GetName() == "Environment") {
                            envEntity = obj;
                            break;
                        }
                    }
                    
                    if (enableIBL) {
                        if (!envEntity) {
                            envEntity = activeWorld->CreateGameObject("Environment");
                        }
                        if (!envEntity->GetComponent<SkyLightComponent>()) {
                            envEntity->AddComponent<SkyLightComponent>();
                        }
                    } else {
                        if (envEntity) {
                            auto skyLight = envEntity->GetComponent<SkyLightComponent>();
                            if (skyLight) {
                                envEntity->RemoveComponent(skyLight);
                            }
                        }
                    }
                }
            }
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Enable Image-Based Lighting using environment maps for realistic ambient lighting.");
            ImGui::EndTooltip();
        }
        
        if (enableIBL) {
            ImGui::Indent();
            
            bool enableDiffuse = m_LightingSettings.EnableDiffuseIBL;
            if (ImGui::Checkbox("Enable Diffuse IBL", &enableDiffuse)) {
                m_LightingSettings.EnableDiffuseIBL = enableDiffuse;
                m_LightingSettingsDirty = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Use environment map for diffuse ambient lighting.");
                ImGui::EndTooltip();
            }
            
            bool enableSpecular = m_LightingSettings.EnableSpecularIBL;
            if (ImGui::Checkbox("Enable Specular IBL", &enableSpecular)) {
                m_LightingSettings.EnableSpecularIBL = enableSpecular;
                m_LightingSettingsDirty = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Use environment map for specular reflections.");
                ImGui::EndTooltip();
            }
            
            ImGui::Unindent();
        }
    }
    
    ImGui::Spacing();
    
    // Advanced Shadow Features
    if (ImGui::CollapsingHeader("Advanced Shadow Features (Future)")) {
        bool enableCascaded = m_LightingSettings.EnableCascadedShadows;
        if (ImGui::Checkbox("Enable Cascaded Shadow Maps (CSM)", &enableCascaded)) {
            m_LightingSettings.EnableCascadedShadows = enableCascaded;
            m_LightingSettingsDirty = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Use multiple shadow maps for near/mid/far distances. Better quality close to camera. (Future feature)");
            ImGui::EndTooltip();
        }
        
        if (enableCascaded) {
            ImGui::Indent();
            ImGui::Text("CSM splits the camera frustum into multiple cascades.");
            ImGui::Text("Each cascade gets its own shadow map for better quality.");
            ImGui::Unindent();
        }
        
        ImGui::Spacing();
        ImGui::Text("Shadow Map Atlas:");
        ImGui::Text("  Combines all shadows into big atlases to reduce bindings.");
        ImGui::Text("  (Future feature - not yet implemented)");
    }
    
    ImGui::Spacing();
    
    // Clustered/Tiled Lighting
    if (ImGui::CollapsingHeader("Clustered/Tiled Lighting (Future)")) {
        ImGui::Text("Clustered/Tiled Lighting:");
        ImGui::Text("  Move from naive N-light loops to clustered/forward+");
        ImGui::Text("  to handle tons of lights efficiently.");
        ImGui::Text("  (Future feature - not yet implemented)");
    }
    
    ImGui::Spacing();
    
    // Light Baking
    if (ImGui::CollapsingHeader("Light Baking & Static Lights (Future)")) {
        ImGui::Text("Light Mobility:");
        ImGui::Text("  - Static: Pre-baked, cannot move or change at runtime");
        ImGui::Text("  - Stationary: Can change intensity/color but not position");
        ImGui::Text("  - Movable: Fully dynamic, can move and change at runtime");
        ImGui::Spacing();
        ImGui::Text("Light Baking:");
        ImGui::Text("  - Pre-bake lightmaps for static lighting");
        ImGui::Text("  - Use light probes / GI volumes");
        ImGui::Text("  (Future feature - not yet implemented)");
    }
    
    ImGui::Spacing();
    
    // Future Features
    if (ImGui::CollapsingHeader("Other Future Features")) {
        bool enableVolumetric = m_LightingSettings.EnableVolumetricFog;
        if (ImGui::Checkbox("Enable Volumetric Fog", &enableVolumetric)) {
            m_LightingSettings.EnableVolumetricFog = enableVolumetric;
            m_LightingSettingsDirty = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Enable volumetric fog rendering. (Future feature - not yet implemented)");
            ImGui::EndTooltip();
        }
        
        bool enableSSAO = m_LightingSettings.EnableSSAO;
        if (ImGui::Checkbox("Enable SSAO", &enableSSAO)) {
            m_LightingSettings.EnableSSAO = enableSSAO;
            m_LightingSettingsDirty = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Enable Screen-Space Ambient Occlusion. (Future feature - not yet implemented)");
            ImGui::EndTooltip();
        }
    }
    
    ImGui::PopID();
}

} // namespace LGE

