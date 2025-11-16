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

#include "LGE/ui/Toolbar.h"
#include "LGE/rendering/Texture.h"
#include "imgui.h"
#include <vector>
#include <string>
#include <cstring>

namespace LGE {

Toolbar::Toolbar()
    : m_IconsLoaded(false)
{
}

Toolbar::~Toolbar() {
}

void Toolbar::LoadIcons() {
    if (m_IconsLoaded) return;
    
    // Load play icon
    m_PlayIcon = std::make_shared<Texture>();
    std::vector<std::string> playPaths = {
        "assets/icons/generic_play_16x.png",
        "unreal-engine-editor-icons/imgs/Icons/generic_play_16x.png"
    };
    bool playLoaded = false;
    for (const auto& path : playPaths) {
        if (m_PlayIcon->LoadImageFile(path)) {
            playLoaded = true;
            break;
        }
    }
    if (!playLoaded) {
        m_PlayIcon.reset();
    }
    
    // Load pause icon
    m_PauseIcon = std::make_shared<Texture>();
    std::vector<std::string> pausePaths = {
        "assets/icons/generic_pause_16x.png",
        "unreal-engine-editor-icons/imgs/Icons/generic_pause_16x.png"
    };
    bool pauseLoaded = false;
    for (const auto& path : pausePaths) {
        if (m_PauseIcon->LoadImageFile(path)) {
            pauseLoaded = true;
            break;
        }
    }
    if (!pauseLoaded) {
        m_PauseIcon.reset();
    }
    
    // Load stop icon
    m_StopIcon = std::make_shared<Texture>();
    std::vector<std::string> stopPaths = {
        "assets/icons/generic_stop_16x.png",
        "unreal-engine-editor-icons/imgs/Icons/generic_stop_16x.png"
    };
    bool stopLoaded = false;
    for (const auto& path : stopPaths) {
        if (m_StopIcon->LoadImageFile(path)) {
            stopLoaded = true;
            break;
        }
    }
    if (!stopLoaded) {
        m_StopIcon.reset();
    }
    
    m_IconsLoaded = true;
}

void Toolbar::OnUIRender() {
    // Load icons on first render
    if (!m_IconsLoaded) {
        LoadIcons();
    }
    // Toolbar style - Modern AAA style
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 3.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 3.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.13f, 1.0f));  // Match modern theme
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    
    // Position toolbar below menu bar (non-resizable)
    // Fixed height to fit content properly
    float menuBarHeight = ImGui::GetFrameHeight();
    float toolbarHeight = 28.0f; // Fixed height that fits content
    
    ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, toolbarHeight), ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoDocking |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    ImGui::Begin("Toolbar", nullptr, flags);
    
    // Center content vertically in toolbar
    float contentHeight = ImGui::GetFrameHeight();
    float verticalOffset = (toolbarHeight - contentHeight) * 0.5f - 2.0f; // Reduce by 2 pixels to bring up
    if (verticalOffset > 0) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + verticalOffset);
    }
    
    // Display current scene name on the left
    float sceneNameWidth = 0.0f;
    if (!m_CurrentSceneName.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Scene:");
        ImGui::SameLine();
        ImGui::Text("%s", m_CurrentSceneName.c_str());
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();
        sceneNameWidth = ImGui::GetCursorPosX();
    }
    
    // Calculate center position for Play, Pause, Stop buttons
    // Use content region to account for padding
    ImVec2 contentRegionMin = ImGui::GetWindowContentRegionMin();
    ImVec2 contentRegionMax = ImGui::GetWindowContentRegionMax();
    float contentWidth = contentRegionMax.x - contentRegionMin.x;
    float buttonGroupWidth = 24.0f * 3.0f + 4.0f * 2.0f; // 3 buttons + 2 spacings
    
    // Center buttons in the remaining space after scene name
    float availableWidth = contentWidth - sceneNameWidth;
    float centerX = contentRegionMin.x + sceneNameWidth + (availableWidth - buttonGroupWidth) * 0.5f;
    
    // Get current Y position to stay on the same line
    float currentY = ImGui::GetCursorPosY();
    
    // Move to center position while preserving Y
    ImGui::SetCursorPos(ImVec2(centerX, currentY));
    
    // Play, Pause, Stop buttons
    ImVec2 buttonSize(24.0f, 24.0f);
    
    // Helper function to draw icon button
    auto DrawIconButton = [this](const char* label, std::shared_ptr<Texture> icon, ImVec2 size) -> bool {
        // Remove all padding and spacing for buttons so background matches button size exactly
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        
        bool clicked = ImGui::Button(label, size);
        
        // Draw icon in the center of the button
        if (ImGui::IsItemVisible()) {
            ImVec2 pos = ImGui::GetItemRectMin();
            ImVec2 rectSize = ImGui::GetItemRectSize();
            ImVec2 center = ImVec2(pos.x + rectSize.x * 0.5f, pos.y + rectSize.y * 0.5f);
            
            if (icon && icon->GetRendererID() != 0) {
                // Use texture icon
                float iconSize = std::min(rectSize.x, rectSize.y) * 0.75f;
                ImVec2 iconMin = ImVec2(center.x - iconSize * 0.5f, center.y - iconSize * 0.5f);
                ImVec2 iconMax = ImVec2(center.x + iconSize * 0.5f, center.y + iconSize * 0.5f);
                ImGui::GetWindowDrawList()->AddImage(
                    reinterpret_cast<void*>(static_cast<intptr_t>(icon->GetRendererID())),
                    iconMin, iconMax,
                    ImVec2(0, 0), ImVec2(1, 1));
            } else {
                // Fallback: draw text
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                if (strcmp(label, "##Play") == 0) {
                    drawList->AddText(center, IM_COL32(255, 255, 255, 255), ">");
                } else if (strcmp(label, "##Pause") == 0) {
                    drawList->AddText(center, IM_COL32(255, 255, 255, 255), "||");
                } else if (strcmp(label, "##Stop") == 0) {
                    drawList->AddText(center, IM_COL32(255, 255, 255, 255), "[]");
                }
            }
        }
        
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(4);
        return clicked;
    };
    
    // Play button
    if (DrawIconButton("##Play", m_PlayIcon, buttonSize)) {
        // Play
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Play");
    }
    
    ImGui::SameLine(0, 4.0f);
    
    // Pause button
    if (DrawIconButton("##Pause", m_PauseIcon, buttonSize)) {
        // Pause
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Pause");
    }
    
    ImGui::SameLine(0, 4.0f);
    
    // Stop button
    if (DrawIconButton("##Stop", m_StopIcon, buttonSize)) {
        // Stop
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Stop");
    }
    
    ImGui::End();
    
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

} // namespace LGE

