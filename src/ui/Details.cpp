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

#include "LGE/ui/Details.h"
#include "imgui.h"

namespace LGE {

Details::Details() {
}

Details::~Details() {
}

void Details::OnUIRender() {
    ImGui::Begin("Details", nullptr);
    
    // Show selected object properties
    ImGui::Text("No object selected");
    ImGui::Separator();
    
    // Transform section
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Location");
        ImGui::PushItemWidth(-1);
        float location[3] = { 0.0f, 0.0f, 0.0f };
        ImGui::InputFloat3("##Location", location);
        ImGui::PopItemWidth();
        
        ImGui::Text("Rotation");
        ImGui::PushItemWidth(-1);
        float rotation[3] = { 0.0f, 0.0f, 0.0f };
        ImGui::InputFloat3("##Rotation", rotation);
        ImGui::PopItemWidth();
        
        ImGui::Text("Scale");
        ImGui::PushItemWidth(-1);
        float scale[3] = { 1.0f, 1.0f, 1.0f };
        ImGui::InputFloat3("##Scale", scale);
        ImGui::PopItemWidth();
    }
    
    // General section
    if (ImGui::CollapsingHeader("General")) {
        ImGui::Text("Name: None");
    }
    
    // Actor section
    if (ImGui::CollapsingHeader("Actor")) {
        ImGui::Text("Actor Class: None");
    }
    
    // Misc section
    if (ImGui::CollapsingHeader("Misc")) {
        ImGui::Text("Tag: None");
    }
    
    // Rendering section
    if (ImGui::CollapsingHeader("Rendering")) {
        ImGui::Text("Visibility: Visible");
    }
    
    ImGui::End();
}

} // namespace LGE


