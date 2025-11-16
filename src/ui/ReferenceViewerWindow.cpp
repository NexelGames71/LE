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

#include "LGE/ui/ReferenceViewerWindow.h"
#include "LGE/core/assets/AssetReferenceFinder.h"
#include "LGE/core/assets/AssetMetadata.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/Log.h"
#include "imgui.h"
#include <algorithm>

namespace LGE {

ReferenceViewerWindow::ReferenceViewerWindow(AssetReferenceFinder* finder)
    : m_Finder(finder)
    , m_CurrentAsset(GUID::Invalid())
    , m_IsOpen(false)
{
}

void ReferenceViewerWindow::Show(const GUID& asset) {
    m_CurrentAsset = asset;
    m_IsOpen = true;
    
    if (m_Finder) {
        m_References = m_Finder->FindReferences(asset);
    } else {
        m_References.clear();
    }
    
    // Update window title
    m_WindowTitle = "Asset References";
}

void ReferenceViewerWindow::Render() {
    if (!m_IsOpen) {
        return;
    }
    
    if (!m_Finder) {
        m_IsOpen = false;
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin(m_WindowTitle.c_str(), &m_IsOpen)) {
        if (m_CurrentAsset.IsValid()) {
            ImGui::Text("Asset GUID: %s", m_CurrentAsset.ToString().c_str());
            ImGui::Separator();
            
            ImGui::Text("References: %zu", m_References.size());
            ImGui::Separator();
            
            if (m_References.empty()) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No references found");
            } else {
                // Table header
                if (ImGui::BeginTable("ReferencesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                    ImGui::TableSetupColumn("Asset Name", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100);
                    ImGui::TableSetupColumn("Reference Type", ImGuiTableColumnFlags_WidthFixed, 120);
                    ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();
                    
                    for (const auto& ref : m_References) {
                        ImGui::TableNextRow();
                        
                        // Asset Name
                        ImGui::TableSetColumnIndex(0);
                        if (ImGui::Selectable(ref.assetName.c_str(), false, ImGuiSelectableFlags_SpanAllColumns)) {
                            // Could open the asset in content browser
                        }
                        
                        // Type
                        ImGui::TableSetColumnIndex(1);
                        std::string typeName;
                        switch (ref.assetType) {
                            case AssetType::Texture: typeName = "Texture"; break;
                            case AssetType::Model: typeName = "Model"; break;
                            case AssetType::Material: typeName = "Material"; break;
                            case AssetType::Shader: typeName = "Shader"; break;
                            case AssetType::Script: typeName = "Script"; break;
                            case AssetType::Audio: typeName = "Audio"; break;
                            case AssetType::Scene: typeName = "Scene"; break;
                            case AssetType::Prefab: typeName = "Prefab"; break;
                            case AssetType::Animation: typeName = "Animation"; break;
                            case AssetType::Font: typeName = "Font"; break;
                            default: typeName = "Unknown"; break;
                        }
                        ImGui::Text("%s", typeName.c_str());
                        
                        // Reference Type
                        ImGui::TableSetColumnIndex(2);
                        const char* refTypeStr = "Direct";
                        switch (ref.referenceType) {
                            case AssetReference::ReferenceType::Direct:
                                refTypeStr = "Direct";
                                break;
                            case AssetReference::ReferenceType::Indirect:
                                refTypeStr = "Indirect";
                                break;
                            case AssetReference::ReferenceType::Scene:
                                refTypeStr = "Scene";
                                break;
                            case AssetReference::ReferenceType::Code:
                                refTypeStr = "Code";
                                break;
                        }
                        ImGui::Text("%s", refTypeStr);
                        
                        // Path
                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", ref.assetPath.c_str());
                    }
                    
                    ImGui::EndTable();
                }
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid asset GUID");
        }
    }
    
    ImGui::End();
    
    if (!m_IsOpen) {
        m_References.clear();
        m_CurrentAsset = GUID::Invalid();
    }
}

} // namespace LGE

