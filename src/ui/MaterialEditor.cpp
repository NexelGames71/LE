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

#include "LGE/ui/MaterialEditor.h"
#include "LGE/rendering/Material.h"
#include "LGE/rendering/Shader.h"
#include "LGE/core/Log.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "imgui.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

namespace LGE {

MaterialEditor::MaterialEditor()
    : m_TextureManager(nullptr)
    , m_Visible(false)
{
    // Initialize available shaders
    m_AvailableShaders = {
        {"Unlit", {"assets/shaders/Unlit.vert", "assets/shaders/Unlit.frag"}},
        {"Skybox", {"assets/shaders/Skybox.vert", "assets/shaders/Skybox.frag"}},
        {"Basic", {"assets/shaders/Basic.vert", "assets/shaders/Basic.frag"}},
        {"Grid", {"assets/shaders/GridMaterial.vert", "assets/shaders/GridMaterial.frag"}}
    };
}

MaterialEditor::~MaterialEditor() {
}

void MaterialEditor::OnUIRender() {
    if (!m_Visible) return;
    
    ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Material Editor", &m_Visible)) {
        if (!m_CurrentMaterial) {
            ImGui::Text("No material selected");
            if (ImGui::Button("Create New Material")) {
                m_CurrentMaterial = std::make_shared<Material>("NewMaterial");
            }
            ImGui::End();
            return;
        }
        
        // Material name
        char nameBuffer[256];
        strncpy_s(nameBuffer, m_CurrentMaterial->GetName().c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        if (ImGui::InputText("Material Name", nameBuffer, sizeof(nameBuffer))) {
            m_CurrentMaterial->SetName(std::string(nameBuffer));
        }
        
        ImGui::Separator();
        
        // Shader selection
        RenderShaderSelection();
        
        ImGui::Separator();
        
        // Parameters
        RenderParameters();
        
        ImGui::Separator();
        
        // Save button
        if (ImGui::Button("Save Material", ImVec2(-1, 0))) {
            SaveMaterial();
        }
    }
    ImGui::End();
}

void MaterialEditor::RenderShaderSelection() {
    ImGui::Text("Shader:");
    
    // Get current shader name
    std::string currentShaderName = "None";
    auto currentShader = m_CurrentMaterial->GetShader();
    if (currentShader) {
        // Try to find matching shader
        for (const auto& shader : m_AvailableShaders) {
            // Check if shader matches (simplified - in production, store shader path in Material)
            currentShaderName = shader.first;
            break; // For now, just use first match
        }
    }
    
    // Shader dropdown
    if (ImGui::BeginCombo("##ShaderSelect", currentShaderName.c_str())) {
        for (const auto& shader : m_AvailableShaders) {
            bool isSelected = (currentShaderName == shader.first);
            if (ImGui::Selectable(shader.first.c_str(), isSelected)) {
                // Load and set shader
                auto newShader = Shader::CreateFromFiles(shader.second.first, shader.second.second);
                if (newShader && newShader->GetRendererID() != 0) {
                    m_CurrentMaterial->SetShader(newShader);
                    Log::Info("Set shader: " + shader.first);
                } else {
                    Log::Error("Failed to load shader: " + shader.first);
                }
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void MaterialEditor::RenderParameters() {
    ImGui::Text("Parameters:");
    
    // Add Parameter button
    if (ImGui::Button("Add Parameter")) {
        ImGui::OpenPopup("AddParameterMenu");
    }
    
    RenderAddParameterMenu();
    
    ImGui::Spacing();
    
    // Render float parameters
    // Note: We need to expose parameter lists from Material class
    // For now, show a simplified version
    
    ImGui::Text("Float Parameters:");
    ImGui::Text("(Parameter editing will be enhanced in next phase)");
    
    ImGui::Spacing();
    ImGui::Text("Vector3 Parameters:");
    ImGui::Text("(Parameter editing will be enhanced in next phase)");
    
    ImGui::Spacing();
    ImGui::Text("Texture Parameters:");
    ImGui::Text("(Parameter editing will be enhanced in next phase)");
}

void MaterialEditor::RenderAddParameterMenu() {
    if (ImGui::BeginPopup("AddParameterMenu")) {
        if (ImGui::MenuItem("Float")) {
            // Add float parameter
            static char paramName[256] = "u_NewFloat";
            if (ImGui::InputText("Parameter Name", paramName, sizeof(paramName), ImGuiInputTextFlags_EnterReturnsTrue)) {
                m_CurrentMaterial->SetFloat(std::string(paramName), 0.0f);
                ImGui::CloseCurrentPopup();
            }
        }
        if (ImGui::MenuItem("Vector3")) {
            // Add Vector3 parameter
            static char paramName[256] = "u_NewVector3";
            if (ImGui::InputText("Parameter Name", paramName, sizeof(paramName), ImGuiInputTextFlags_EnterReturnsTrue)) {
                m_CurrentMaterial->SetVector3(std::string(paramName), Math::Vector3(0.0f, 0.0f, 0.0f));
                ImGui::CloseCurrentPopup();
            }
        }
        if (ImGui::MenuItem("Texture")) {
            // Add texture parameter
            static char paramName[256] = "u_NewTexture";
            if (ImGui::InputText("Parameter Name", paramName, sizeof(paramName), ImGuiInputTextFlags_EnterReturnsTrue)) {
                // For now, set to nullptr - user can assign texture later
                m_CurrentMaterial->SetTexture(std::string(paramName), nullptr);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
}

void MaterialEditor::SaveMaterial() {
    if (!m_CurrentMaterial) {
        Log::Warn("No material to save");
        return;
    }
    
    // Get materials directory and ensure it exists
    std::string materialsDir = FileSystem::GetMaterialsDirectory();
    if (!FileSystem::Exists(materialsDir)) {
        FileSystem::CreateDirectory(materialsDir);
    }
    
    // Generate material asset file path
    std::string materialName = m_CurrentMaterial->GetName();
    std::string assetPath = FileSystem::JoinPath(materialsDir, materialName + ".material");
    
    // Build JSON content
    std::stringstream json;
    json << "{\n";
    json << "  \"name\": \"" << materialName << "\",\n";
    
    // Save shader path (simplified - in production, store shader reference)
    auto shader = m_CurrentMaterial->GetShader();
    if (shader) {
        json << "  \"shader\": \"PBR\",\n";  // Simplified
    } else {
        json << "  \"shader\": \"None\",\n";
    }
    
    json << "  \"parameters\": {\n";
    json << "    \"_comment\": \"Parameters will be saved in next phase\"\n";
    json << "  }\n";
    json << "}\n";
    
    // Save using FileSystem
    if (FileSystem::WriteFile(assetPath, json.str())) {
        Log::Info("Saved material: " + assetPath);
    } else {
        Log::Error("Failed to save material: " + assetPath);
    }
}

} // namespace LGE

