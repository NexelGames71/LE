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

#include "LGE/ui/ContentBrowser.h"
#include "imgui.h"

namespace LGE {

ContentBrowser::ContentBrowser() {
}

ContentBrowser::~ContentBrowser() {
}

void ContentBrowser::OnUIRender() {
    ImGui::Begin("Content Browser", nullptr);
    
    // Toolbar matching Unreal Engine
    if (ImGui::Button("Add")) {
        // Add asset
    }
    ImGui::SameLine();
    if (ImGui::Button("Import")) {
        // Import asset
    }
    ImGui::SameLine();
    if (ImGui::Button("Save All")) {
        // Save all assets
    }
    ImGui::Separator();
    ImGui::SameLine();
    if (ImGui::Button("Import")) {
        // Import asset
    }
    ImGui::SameLine();
    if (ImGui::Button("Save All")) {
        // Save all
    }
    
    ImGui::Separator();
    
    // Split view: folders on left, content on right
    float panelWidth = ImGui::GetContentRegionAvail().x;
    float folderPanelWidth = panelWidth * 0.25f;
    
    // Folder tree
    ImGui::BeginChild("FolderTree", ImVec2(folderPanelWidth, 0), false);
    if (ImGui::TreeNode("Assets")) {
        if (ImGui::TreeNode("Shaders")) {
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("HDR Sky")) {
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("EXR Sky")) {
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Content area
    ImGui::BeginChild("ContentArea", ImVec2(0, 0), false);
    ImGui::Text("Content area - assets will be displayed here");
    ImGui::EndChild();
    
    ImGui::End();
}

} // namespace LGE

