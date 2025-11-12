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

// Include OpenGL before other headers
#include <glad/glad.h>

// Standard library headers must come before LGE headers to avoid namespace collisions
#include <functional>
#include <cmath>
#include <algorithm>
#include <vector>

// Include these BEFORE SceneViewport.h to avoid namespace collision
// (They need to open namespace LGE at global scope, not inside SceneViewport.h's namespace)
#include "LGE/rendering/Camera.h"
#include "LGE/rendering/Renderer.h"
#include "LGE/ui/SceneViewport.h"
#include "LGE/rendering/Texture.h"
#include "LGE/rendering/Shader.h"
#include "LGE/rendering/VertexArray.h"
#include "LGE/rendering/VertexBuffer.h"
#include "LGE/core/FileSystem.h"
#include "LGE/core/Log.h"
#include "LGE/core/Input.h"
#include "LGE/core/GameObject.h"
#include "LGE/ui/UI.h"
#include "LGE/rendering/Luminite/LuminiteSubsystem.h"
#include "imgui.h"
#include "ImGuizmo.h"

namespace LGE {

void SceneViewport::SetLuminiteSubsystem(Luminite::LuminiteSubsystem* subsystem) {
    m_LuminiteSubsystem = subsystem;
}

SceneViewport::SceneViewport()
    : m_Camera(nullptr)
    , m_Width(1280)
    , m_Height(720)
    , m_Focused(false)
    , m_Hovered(false)
    , m_ViewportSizeChanged(false)
    , m_IconsLoaded(false)
    , m_SelectedObject(nullptr)
    , m_SelectedTool(0)
    , m_IsDragging(false)
    , m_ShowGrid(true)
    , m_IsLit(true)
    , m_ProjectionType(0) // 0=Perspective, 1=Orthographic
    , m_LuminiteSubsystem(nullptr)
{
    // Don't create framebuffer here - wait until OpenGL is initialized
    // Framebuffer will be created on first render
    m_StoredViewport[0] = 0;
    m_StoredViewport[1] = 0;
    m_StoredViewport[2] = 1280;
    m_StoredViewport[3] = 720;
}

SceneViewport::~SceneViewport() {
}

void SceneViewport::OnUpdate(float deltaTime) {
    m_ViewportSizeChanged = false;
}

void SceneViewport::BeginRender() {
    // Don't render if viewport is too small
    if (m_Width == 0 || m_Height == 0) {
        return;
    }
    
    // Create framebuffer if it doesn't exist (OpenGL should be initialized by now)
    if (!m_Framebuffer) {
        m_Framebuffer = std::make_unique<Framebuffer>(m_Width, m_Height);
    }
    
    if (!m_Framebuffer) {
        return;
    }
    
    // Store current viewport to restore later
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    m_StoredViewport[0] = viewport[0];
    m_StoredViewport[1] = viewport[1];
    m_StoredViewport[2] = viewport[2];
    m_StoredViewport[3] = viewport[3];
    
    // Render scene to framebuffer
    m_Framebuffer->Bind();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Scene rendering will happen here (called from application)
}

void SceneViewport::EndRender() {
    if (!m_Framebuffer) {
        return;
    }
    
    // Unbind framebuffer
    m_Framebuffer->Unbind();
    
    // Restore viewport to what it was before
    glViewport(m_StoredViewport[0], m_StoredViewport[1], m_StoredViewport[2], m_StoredViewport[3]);
}

void SceneViewport::LoadIcons() {
    if (m_IconsLoaded) return;
    
    // Try to load Unreal Engine editor icons (EditorViewport transform tools)
    std::vector<std::string> translatePaths = {
        "assets/icons/EditorViewport.TranslateMode.png",
        "assets/icons/EditorViewport.TranslateMode.Small.png",
        "assets/icons/icon_translate_40x.png",
        "assets/icons/icon_translate_24x.png",
        "assets/icons/icon_translate_16x.png",
        "assets/icons/translate.png"
    };
    
    std::vector<std::string> rotatePaths = {
        "assets/icons/EditorViewport.RotateMode.png",
        "assets/icons/EditorViewport.RotateMode.Small.png",
        "assets/icons/icon_rotate_40x.png",
        "assets/icons/icon_rotate_24x.png",
        "assets/icons/icon_rotate_16x.png",
        "assets/icons/rotate.png"
    };
    
    std::vector<std::string> scalePaths = {
        "assets/icons/EditorViewport.ScaleMode.png",
        "assets/icons/EditorViewport.ScaleMode.Small.png",
        "assets/icons/icon_scale_40x.png",
        "assets/icons/icon_scale_24x.png",
        "assets/icons/icon_scale_16x.png",
        "assets/icons/scale.png"
    };
    
    std::vector<std::string> showGridPaths = {
        "assets/icons/icon_ShowGrid_16x.png",
        "unreal-engine-editor-icons/imgs/Icons/icon_ShowGrid_16x.png"
    };
    
    std::vector<std::string> litIconPaths = {
        "unreal-engine-editor-icons/imgs/Icons/icon_ViewMode_Lit_16px.png",
        "assets/icons/icon_ViewMode_Lit_16px.png"
    };
    
    // Load translate icon
    m_TranslateIcon = std::make_shared<Texture>();
    bool translateLoaded = false;
    for (const auto& path : translatePaths) {
        if (m_TranslateIcon->LoadImageFile(path)) {
            translateLoaded = true;
            break;
        }
    }
    if (!translateLoaded) {
        m_TranslateIcon.reset(); // Failed to load, will use fallback
    }
    
    // Load rotate icon
    m_RotateIcon = std::make_shared<Texture>();
    bool rotateLoaded = false;
    for (const auto& path : rotatePaths) {
        if (m_RotateIcon->LoadImageFile(path)) {
            rotateLoaded = true;
            break;
        }
    }
    if (!rotateLoaded) {
        m_RotateIcon.reset(); // Failed to load, will use fallback
    }
    
    // Load scale icon
    m_ScaleIcon = std::make_shared<Texture>();
    bool scaleLoaded = false;
    for (const auto& path : scalePaths) {
        if (m_ScaleIcon->LoadImageFile(path)) {
            scaleLoaded = true;
            break;
        }
    }
    if (!scaleLoaded) {
        m_ScaleIcon.reset(); // Failed to load, will use fallback
    }
    
    // Load show grid icon
    m_ShowGridIcon = std::make_shared<Texture>();
    bool showGridLoaded = false;
    for (const auto& path : showGridPaths) {
        if (m_ShowGridIcon->LoadImageFile(path)) {
            showGridLoaded = true;
            break;
        }
    }
    if (!showGridLoaded) {
        m_ShowGridIcon.reset(); // Failed to load, will use fallback
    }
    
    // Load lit icon
    m_LitIcon = std::make_shared<Texture>();
    bool litIconLoaded = false;
    for (const auto& path : litIconPaths) {
        if (m_LitIcon->LoadImageFile(path)) {
            litIconLoaded = true;
            break;
        }
    }
    if (!litIconLoaded) {
        m_LitIcon.reset(); // Failed to load, will use fallback
    }
    
    // Load light actor icon
    m_LightActorIcon = std::make_shared<Texture>();
    std::vector<std::string> lightActorPaths = {
        "unreal-engine-editor-icons/imgs/Icons/ActorIcons/LightActor_16x.png",
        "assets/icons/LightActor_16x.png"
    };
    bool lightActorLoaded = false;
    for (const auto& path : lightActorPaths) {
        if (m_LightActorIcon->LoadImageFile(path)) {
            lightActorLoaded = true;
            break;
        }
    }
    if (!lightActorLoaded) {
        m_LightActorIcon.reset();
    }
    
    m_IconsLoaded = true;
}

void SceneViewport::OnUIRender() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    // Make window background transparent for toolbar
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::Begin("Scene Viewport", nullptr);

    m_Focused = ImGui::IsWindowFocused();
    m_Hovered = ImGui::IsWindowHovered();

    // Load icons on first render
    if (!m_IconsLoaded) {
        LoadIcons();
    }

    // Viewport toolbar above viewport (Unreal Engine style)
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent frame background
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent popup background
    
    ImGui::BeginChild("ViewportToolbar", ImVec2(0, 28.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
    
    // Viewport options
    // Perspective/Orthographic dropdown
    const char* projectionTypes[] = { "Perspective", "Orthographic" };
    ImGui::PushItemWidth(100.0f); // Set width for the combo
    
    // Adjust vertical position to move dropdown down
    ImVec2 currentPos = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(currentPos.x, currentPos.y + 4.0f));
    
    if (ImGui::BeginCombo("##ProjectionType", projectionTypes[m_ProjectionType], ImGuiComboFlags_NoArrowButton)) {
        for (int i = 0; i < 2; i++) {
            bool isSelected = (m_ProjectionType == i);
            if (ImGui::Selectable(projectionTypes[i], isSelected)) {
                m_ProjectionType = i;
                // Update camera projection
                if (m_Camera) {
                    float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
                    if (m_ProjectionType == 0) {
                        // Perspective
                        m_Camera->SetPerspective(45.0f, aspectRatio, 0.1f, 100.0f);
                    } else {
                        // Orthographic - use smaller size to see skybox better
                        float orthoSize = 3.0f; // Smaller size to match perspective view and see skybox
                        m_Camera->SetOrthographic(-orthoSize * aspectRatio, orthoSize * aspectRatio, 
                                                  -orthoSize, orthoSize, 0.1f, 100.0f);
                    }
                }
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    
    // Lit toggle button with icon
    ImGui::PushStyleColor(ImGuiCol_Button, m_IsLit ? ImVec4(0.3f, 0.5f, 0.8f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
    
    // Adjust vertical position to move button down
    ImVec2 litButtonPos = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(litButtonPos.x, litButtonPos.y + 4.0f));
    
    if (ImGui::Button("##LitToggle", ImVec2(28.0f, 20.0f))) {
        m_IsLit = !m_IsLit;
    }
    
    // Draw lit icon in the center of the button
    if (ImGui::IsItemVisible()) {
        ImVec2 pos = ImGui::GetItemRectMin();
        ImVec2 rectSize = ImGui::GetItemRectSize();
        ImVec2 center = ImVec2(pos.x + rectSize.x * 0.5f, pos.y + rectSize.y * 0.5f);
        
        if (m_LitIcon && m_LitIcon->GetRendererID() != 0) {
            // Use texture icon
            float iconSize = std::min(rectSize.x, rectSize.y) * 0.7f;
            ImVec2 iconMin = ImVec2(center.x - iconSize * 0.5f, center.y - iconSize * 0.5f);
            ImVec2 iconMax = ImVec2(center.x + iconSize * 0.5f, center.y + iconSize * 0.5f);
            ImGui::GetWindowDrawList()->AddImage(
                reinterpret_cast<void*>(static_cast<intptr_t>(m_LitIcon->GetRendererID())),
                iconMin, iconMax,
                ImVec2(0, 0), ImVec2(1, 1));
        } else {
            // Fallback: draw a simple lightbulb icon
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImU32 color = m_IsLit ? IM_COL32(255, 255, 200, 255) : IM_COL32(128, 128, 128, 255);
            float radius = 6.0f;
            drawList->AddCircleFilled(center, radius, color);
            drawList->AddLine(ImVec2(center.x, center.y - radius * 1.5f), ImVec2(center.x, center.y - radius * 2.5f), color, 2.0f);
        }
    }
    
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(m_IsLit ? "Disable Lighting" : "Enable Lighting");
    }
    
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Transform tool icons
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 0.0f));
    
    // Helper function to draw icon button with texture or fallback
    auto DrawIconButton = [this](const char* label, bool selected, ImVec2 size, std::shared_ptr<Texture> icon, std::function<void(ImVec2, ImVec2)> fallbackDraw) -> bool {
        ImGui::PushStyleColor(ImGuiCol_Button, selected ? ImVec4(0.3f, 0.5f, 0.8f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
        
        // Use unique label with ## prefix (## makes it invisible but gives unique ID)
        std::string uniqueLabel = std::string("##") + label;
        bool clicked = ImGui::Button(uniqueLabel.c_str(), size);
        
        // Draw icon in the center of the button
        if (ImGui::IsItemVisible()) {
            ImVec2 pos = ImGui::GetItemRectMin();
            ImVec2 rectSize = ImGui::GetItemRectSize();
            ImVec2 center = ImVec2(pos.x + rectSize.x * 0.5f, pos.y + rectSize.y * 0.5f);
            
            if (icon && icon->GetRendererID() != 0) {
                // Use texture icon
                float iconSize = std::min(rectSize.x, rectSize.y) * 0.7f;
                ImVec2 iconMin = ImVec2(center.x - iconSize * 0.5f, center.y - iconSize * 0.5f);
                ImVec2 iconMax = ImVec2(center.x + iconSize * 0.5f, center.y + iconSize * 0.5f);
                ImGui::GetWindowDrawList()->AddImage(
                    reinterpret_cast<void*>(static_cast<intptr_t>(icon->GetRendererID())),
                    iconMin, iconMax,
                    ImVec2(0, 0), ImVec2(1, 1)); // Normal texture coordinates
            } else {
                // Use fallback drawn icon
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                fallbackDraw(center, rectSize);
            }
        }
        
        ImGui::PopStyleColor(3);
        return clicked;
    };
    
    // Translate icon
    if (DrawIconButton("Translate", m_SelectedTool == 0, ImVec2(36.0f, 32.0f), m_TranslateIcon, [](ImVec2 center, ImVec2 size) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 color = IM_COL32(255, 255, 255, 255);
        float radius = 6.0f;
        drawList->AddLine(ImVec2(center.x + radius, center.y), ImVec2(center.x - radius, center.y), color, 1.5f);
        drawList->AddLine(ImVec2(center.x + radius * 0.5f, center.y - radius * 0.5f), ImVec2(center.x + radius, center.y), color, 1.5f);
        drawList->AddLine(ImVec2(center.x + radius * 0.5f, center.y + radius * 0.5f), ImVec2(center.x + radius, center.y), color, 1.5f);
        drawList->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), color, 1.5f);
        drawList->AddLine(ImVec2(center.x - radius * 0.5f, center.y - radius * 0.5f), ImVec2(center.x, center.y - radius), color, 1.5f);
        drawList->AddLine(ImVec2(center.x + radius * 0.5f, center.y - radius * 0.5f), ImVec2(center.x, center.y - radius), color, 1.5f);
        drawList->AddCircleFilled(center, 2.0f, color);
    })) {
        m_SelectedTool = 0;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Translate (W)");
    }
    
    ImGui::SameLine();
    
    // Rotate icon
    if (DrawIconButton("Rotate", m_SelectedTool == 1, ImVec2(36.0f, 32.0f), m_RotateIcon, [](ImVec2 center, ImVec2 size) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 color = IM_COL32(255, 255, 255, 255);
        float radius = 7.0f;
        const int num_segments = 20;
        for (int i = 0; i < num_segments * 3 / 4; i++) {
            float angle1 = (i / (float)num_segments) * 2.0f * 3.14159f - 3.14159f * 0.5f;
            float angle2 = ((i + 1) / (float)num_segments) * 2.0f * 3.14159f - 3.14159f * 0.5f;
            ImVec2 p1 = ImVec2(center.x + std::cos(angle1) * radius, center.y + std::sin(angle1) * radius);
            ImVec2 p2 = ImVec2(center.x + std::cos(angle2) * radius, center.y + std::sin(angle2) * radius);
            drawList->AddLine(p1, p2, color, 1.5f);
        }
        float arrowAngle = (3.0f / 4.0f) * 2.0f * 3.14159f - 3.14159f * 0.5f;
        ImVec2 arrowTip = ImVec2(center.x + std::cos(arrowAngle) * radius, center.y + std::sin(arrowAngle) * radius);
        ImVec2 arrowLeft = ImVec2(arrowTip.x - std::cos(arrowAngle + 0.5f) * 3.0f, arrowTip.y - std::sin(arrowAngle + 0.5f) * 3.0f);
        ImVec2 arrowRight = ImVec2(arrowTip.x - std::cos(arrowAngle - 0.5f) * 3.0f, arrowTip.y - std::sin(arrowAngle - 0.5f) * 3.0f);
        drawList->AddTriangleFilled(arrowTip, arrowLeft, arrowRight, color);
    })) {
        m_SelectedTool = 1;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Rotate (E)");
    }
    
    ImGui::SameLine();
    
    // Scale icon
    if (DrawIconButton("Scale", m_SelectedTool == 2, ImVec2(36.0f, 32.0f), m_ScaleIcon, [](ImVec2 center, ImVec2 size) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 color = IM_COL32(255, 255, 255, 255);
        float boxSize = 8.0f;
        ImVec2 boxMin = ImVec2(center.x - boxSize * 0.5f, center.y - boxSize * 0.5f);
        ImVec2 boxMax = ImVec2(center.x + boxSize * 0.5f, center.y + boxSize * 0.5f);
        drawList->AddRect(boxMin, boxMax, color, 0.0f, 0, 1.5f);
        float handleSize = 3.0f;
        drawList->AddRectFilled(ImVec2(boxMin.x - handleSize * 0.5f, boxMin.y - handleSize * 0.5f), ImVec2(boxMin.x + handleSize * 0.5f, boxMin.y + handleSize * 0.5f), color);
        drawList->AddRectFilled(ImVec2(boxMax.x - handleSize * 0.5f, boxMin.y - handleSize * 0.5f), ImVec2(boxMax.x + handleSize * 0.5f, boxMin.y + handleSize * 0.5f), color);
        drawList->AddRectFilled(ImVec2(boxMin.x - handleSize * 0.5f, boxMax.y - handleSize * 0.5f), ImVec2(boxMin.x + handleSize * 0.5f, boxMax.y + handleSize * 0.5f), color);
        drawList->AddRectFilled(ImVec2(boxMax.x - handleSize * 0.5f, boxMax.y - handleSize * 0.5f), ImVec2(boxMax.x + handleSize * 0.5f, boxMax.y + handleSize * 0.5f), color);
    })) {
        m_SelectedTool = 2;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Scale (R)");
    }
    
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Show Grid toggle button
    ImGui::PushStyleColor(ImGuiCol_Button, m_ShowGrid ? ImVec4(0.3f, 0.5f, 0.8f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
    
    if (ImGui::Button("##ShowGrid", ImVec2(36.0f, 32.0f))) {
        m_ShowGrid = !m_ShowGrid;
    }
    
    // Draw icon in the center of the button
    if (ImGui::IsItemVisible()) {
        ImVec2 pos = ImGui::GetItemRectMin();
        ImVec2 rectSize = ImGui::GetItemRectSize();
        ImVec2 center = ImVec2(pos.x + rectSize.x * 0.5f, pos.y + rectSize.y * 0.5f);
        
        if (m_ShowGridIcon && m_ShowGridIcon->GetRendererID() != 0) {
            // Use texture icon
            float iconSize = std::min(rectSize.x, rectSize.y) * 0.7f;
            ImVec2 iconMin = ImVec2(center.x - iconSize * 0.5f, center.y - iconSize * 0.5f);
            ImVec2 iconMax = ImVec2(center.x + iconSize * 0.5f, center.y + iconSize * 0.5f);
            ImGui::GetWindowDrawList()->AddImage(
                reinterpret_cast<void*>(static_cast<intptr_t>(m_ShowGridIcon->GetRendererID())),
                iconMin, iconMax,
                ImVec2(0, 0), ImVec2(1, 1));
        } else {
            // Fallback: draw grid pattern
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImU32 color = IM_COL32(255, 255, 255, 255);
            float size = 8.0f;
            // Draw grid pattern
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    ImVec2 cellPos = ImVec2(center.x + i * size * 0.5f, center.y + j * size * 0.5f);
                    drawList->AddRect(cellPos, ImVec2(cellPos.x + size * 0.3f, cellPos.y + size * 0.3f), color, 0.0f, 0, 1.0f);
                }
            }
        }
    }
    
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(m_ShowGrid ? "Hide Grid" : "Show Grid");
    }
    
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Snap settings (matching Unreal Engine)
    ImGui::Text("10");
    ImGui::SameLine();
    ImGui::Text("10°");
    ImGui::SameLine();
    ImGui::Text("0.25");
    ImGui::SameLine();
    ImGui::Text("1");
    
    ImGui::PopStyleVar(2);
    ImGui::EndChild();
    ImGui::PopStyleColor(3); // Pop ChildBg, FrameBg, and PopupBg colors
    ImGui::PopStyleVar(2);
    
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    
    // Ensure minimum size
    if (viewportPanelSize.x < 1.0f) viewportPanelSize.x = 1.0f;
    if (viewportPanelSize.y < 1.0f) viewportPanelSize.y = 1.0f;
    
    if (viewportPanelSize.x != m_Width || viewportPanelSize.y != m_Height) {
        m_Width = static_cast<uint32_t>(viewportPanelSize.x);
        m_Height = static_cast<uint32_t>(viewportPanelSize.y);
        m_ViewportSizeChanged = true;
        
        if (m_Width > 0 && m_Height > 0) {
            if (m_Framebuffer) {
                m_Framebuffer->Resize(m_Width, m_Height);
            }
            
            // Update camera aspect ratio
            if (m_Camera) {
                float aspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
                if (m_ProjectionType == 0) {
                    // Perspective
                    m_Camera->SetPerspective(45.0f, aspectRatio, 0.1f, 100.0f);
                } else {
                    // Orthographic - use smaller size to see skybox better
                    float orthoSize = 3.0f; // Smaller size to match perspective view and see skybox
                    m_Camera->SetOrthographic(-orthoSize * aspectRatio, orthoSize * aspectRatio, 
                                              -orthoSize, orthoSize, 0.1f, 100.0f);
                }
            }
        }
    }

    // Display the framebuffer texture
    if (m_Framebuffer && m_Width > 0 && m_Height > 0) {
        uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImVec2 imagePos = ImGui::GetCursorScreenPos();
        ImGui::Image(reinterpret_cast<void*>(textureID), viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));
        
        // Set up ImGuizmo
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, 
                         ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
        
        // Render GameObject icons (e.g., DirectionalLight icons)
        if (m_Camera && m_LightActorIcon && m_LightActorIcon->GetRendererID() != 0) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const Math::Matrix4& viewProj = m_Camera->GetViewProjectionMatrix();
            float iconSize = 32.0f; // Size of icon in pixels
            
            for (const auto& obj : m_GameObjects) {
                if (!obj) continue;
                
                // Check if this is a DirectionalLight GameObject
                bool isLight = (obj->GetName().find("Directional Light") != std::string::npos);
                if (!isLight) continue;
                
                // Get object position
                Math::Vector3 worldPos = obj->GetPosition();
                
                // Project 3D position to screen space
                Math::Vector4 clipPos = viewProj * Math::Vector4(worldPos.x, worldPos.y, worldPos.z, 1.0f);
                
                // Perspective divide
                if (std::abs(clipPos.w) > 0.0001f) {
                    float ndcX = clipPos.x / clipPos.w;
                    float ndcY = clipPos.y / clipPos.w;
                    float ndcZ = clipPos.z / clipPos.w;
                    
                    // Check if point is in front of camera and within view frustum
                    if (ndcZ > -1.0f && ndcZ < 1.0f && 
                        ndcX > -1.0f && ndcX < 1.0f && 
                        ndcY > -1.0f && ndcY < 1.0f) {
                        
                        // Convert NDC to screen coordinates
                        // ImGui uses top-left origin, OpenGL uses bottom-left
                        float screenX = imagePos.x + (ndcX + 1.0f) * 0.5f * viewportPanelSize.x;
                        float screenY = imagePos.y + (1.0f - ndcY) * 0.5f * viewportPanelSize.y;
                        
                        // Draw icon centered at screen position
                        ImVec2 iconMin = ImVec2(screenX - iconSize * 0.5f, screenY - iconSize * 0.5f);
                        ImVec2 iconMax = ImVec2(screenX + iconSize * 0.5f, screenY + iconSize * 0.5f);
                        
                        drawList->AddImage(
                            reinterpret_cast<void*>(static_cast<intptr_t>(m_LightActorIcon->GetRendererID())),
                            iconMin, iconMax,
                            ImVec2(0, 0), ImVec2(1, 1)
                        );
                    }
                }
            }
        }
        
        // Grid is now rendered using GridRenderer in main.cpp (respects depth, doesn't show through objects)
        
        // Render ImGuizmo if object is selected
        if (m_SelectedObject && m_Camera) {
            RenderImGuizmo();
        }
        
        // Handle mouse input for selection (only when viewport is focused and hovered)
        if (m_Focused && m_Hovered && !UI::WantCaptureMouse() && !ImGuizmo::IsOver()) {
            HandleMouseInput();
        }
    }

    ImGui::End();
    ImGui::PopStyleColor(); // Pop window background color
    ImGui::PopStyleVar();
}

void SceneViewport::HandleMouseInput() {
    if (!m_Camera) return;
    
    // Don't process mouse input if middle mouse is pressed (camera panning)
    // Middle mouse is handled by CameraController
    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        m_IsDragging = false;
        return;
    }
    
    // Get mouse position in viewport
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 viewportMin = ImGui::GetItemRectMin();
    ImVec2 viewportMax = ImGui::GetItemRectMax();
    
    // Check if mouse is within viewport
    if (mousePos.x < viewportMin.x || mousePos.x > viewportMax.x ||
        mousePos.y < viewportMin.y || mousePos.y > viewportMax.y) {
        return;
    }
    
    // Convert to viewport coordinates (0,0 at top-left)
    float viewportX = mousePos.x - viewportMin.x;
    float viewportY = mousePos.y - viewportMin.y;
    
    // Convert to normalized device coordinates (-1 to 1, with Y flipped)
    float ndcX = (viewportX / m_Width) * 2.0f - 1.0f;
    float ndcY = 1.0f - (viewportY / m_Height) * 2.0f;
    
    bool leftMousePressed = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    static bool leftMouseWasPressed = false;
    
    // Handle left mouse button for selection
    if (leftMousePressed && !leftMouseWasPressed) {
        // Mouse just clicked - perform object selection
        // Simple selection: check if clicking near any GameObject
        // For now, we'll use a simple screen-space check
        // TODO: Implement proper raycasting for accurate object selection
        
        bool objectSelected = false;
        
        // Check all GameObjects to see if we clicked on one
        for (auto& obj : m_GameObjects) {
            if (!obj) continue;
            
            // Get object position in world space
            Math::Vector3 objPos = obj->GetPosition();
            
            // Project object position to screen space
            // This is a simplified check - proper implementation would use raycasting
            Math::Vector3 viewPos = m_Camera->GetPosition();
            Math::Vector3 toObject = Math::Vector3(
                objPos.x - viewPos.x,
                objPos.y - viewPos.y,
                objPos.z - viewPos.z
            );
            
            // Simple distance check - if object is roughly in front of camera
            float distance = std::sqrt(toObject.x * toObject.x + toObject.y * toObject.y + toObject.z * toObject.z);
            
            // If clicking anywhere on screen and object is in front of camera, select it
            // This is a simplified selection - proper implementation would use proper projection/raycasting
            // For now, if object is within reasonable distance, select it when clicking anywhere
            if (distance < 20.0f) {
                // Select this object (first one found that's close enough)
                m_SelectedObject = obj.get();
                obj->SetSelected(true);
                
                // Deselect others
                for (auto& other : m_GameObjects) {
                    if (other.get() != obj.get()) {
                        other->SetSelected(false);
                    }
                }
                
                objectSelected = true;
                Log::Info("Selected object: " + obj->GetName() + " at click position: " + 
                         std::to_string(ndcX) + ", " + std::to_string(ndcY) + " distance: " + std::to_string(distance));
                break;
            }
        }
        
        // If we selected an object, prepare for dragging
        if (m_SelectedObject) {
            m_DragStartPos = Math::Vector3(ndcX, ndcY, 0.0f);
            m_DragStartObjectPos = m_SelectedObject->GetPosition();
            m_DragStartObjectRot = m_SelectedObject->GetRotation();
            m_DragStartObjectScale = m_SelectedObject->GetScale();
            m_IsDragging = true;
        } else if (!objectSelected) {
            // Clicked on empty space - deselect
            m_SelectedObject = nullptr;
            for (auto& obj : m_GameObjects) {
                if (obj) {
                    obj->SetSelected(false);
                }
            }
        }
    } else if (leftMousePressed && leftMouseWasPressed && m_IsDragging && m_SelectedObject) {
        // Mouse is being dragged - perform transform via gizmo
        // TODO: Implement proper gizmo-based transformation with axis constraints
        Math::Vector3 currentMousePos(ndcX, ndcY, 0.0f);
        Math::Vector3 delta = Math::Vector3(
            currentMousePos.x - m_DragStartPos.x,
            currentMousePos.y - m_DragStartPos.y,
            0.0f
        );
        
        // Transform based on selected tool
        if (m_SelectedTool == 0) { // Translate
            // Calculate movement in world space
            float moveSpeed = 2.0f;
            Math::Vector3 worldDelta = Math::Vector3(
                delta.x * moveSpeed,
                -delta.y * moveSpeed, // Invert Y
                0.0f
            );
            m_SelectedObject->SetPosition(Math::Vector3(
                m_DragStartObjectPos.x + worldDelta.x,
                m_DragStartObjectPos.y + worldDelta.y,
                m_DragStartObjectPos.z + worldDelta.z
            ));
        } else if (m_SelectedTool == 1) { // Rotate
            float rotateSpeed = 90.0f; // degrees per unit of mouse movement
            m_SelectedObject->SetRotation(Math::Vector3(
                m_DragStartObjectRot.x,
                m_DragStartObjectRot.y + delta.x * rotateSpeed,
                m_DragStartObjectRot.z
            ));
        } else if (m_SelectedTool == 2) { // Scale
            float scaleSpeed = 1.0f;
            float scaleDelta = (delta.x + delta.y) * scaleSpeed;
            Math::Vector3 newScale = Math::Vector3(
                m_DragStartObjectScale.x + scaleDelta,
                m_DragStartObjectScale.y + scaleDelta,
                m_DragStartObjectScale.z + scaleDelta
            );
            // Clamp scale to prevent negative values
            newScale.x = std::max(0.1f, newScale.x);
            newScale.y = std::max(0.1f, newScale.y);
            newScale.z = std::max(0.1f, newScale.z);
            m_SelectedObject->SetScale(newScale);
        }
    } else if (!leftMousePressed && leftMouseWasPressed) {
        // Mouse released
        m_IsDragging = false;
    }
    
    leftMouseWasPressed = leftMousePressed;
}

void SceneViewport::RenderImGuizmo() {
    if (!m_SelectedObject || !m_Camera) {
        return;
    }
    
    // Get camera matrices
    const Math::Matrix4& viewMatrix = m_Camera->GetViewMatrix();
    const Math::Matrix4& projectionMatrix = m_Camera->GetProjectionMatrix();
    
    // Convert matrices to float arrays (column-major for OpenGL)
    float view[16];
    float projection[16];
    float objectMatrix[16];
    
    // Copy view matrix
    const float* viewData = viewMatrix.GetData();
    for (int i = 0; i < 16; i++) {
        view[i] = viewData[i];
    }
    
    // Copy projection matrix
    const float* projData = projectionMatrix.GetData();
    for (int i = 0; i < 16; i++) {
        projection[i] = projData[i];
    }
    
    // Get object transform matrix
    const Math::Matrix4& transformMatrix = m_SelectedObject->GetTransformMatrix();
    const float* transformData = transformMatrix.GetData();
    for (int i = 0; i < 16; i++) {
        objectMatrix[i] = transformData[i];
    }
    
    // Determine operation based on selected tool
    ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
    if (m_SelectedTool == 1) {
        operation = ImGuizmo::ROTATE;
    } else if (m_SelectedTool == 2) {
        operation = ImGuizmo::SCALE;
    }
    
    // Use local mode
    ImGuizmo::MODE mode = ImGuizmo::LOCAL;
    
    // Manipulate the object
    bool manipulated = ImGuizmo::Manipulate(view, projection, operation, mode, objectMatrix);
    
    // If the gizmo was manipulated, update the GameObject
    if (manipulated) {
        // Decompose the matrix to get position, rotation, and scale
        Math::Vector3 translation, rotation, scale;
        
        // Extract translation
        translation.x = objectMatrix[12];
        translation.y = objectMatrix[13];
        translation.z = objectMatrix[14];
        
        // Extract scale (length of each column)
        scale.x = std::sqrt(objectMatrix[0] * objectMatrix[0] + objectMatrix[1] * objectMatrix[1] + objectMatrix[2] * objectMatrix[2]);
        scale.y = std::sqrt(objectMatrix[4] * objectMatrix[4] + objectMatrix[5] * objectMatrix[5] + objectMatrix[6] * objectMatrix[6]);
        scale.z = std::sqrt(objectMatrix[8] * objectMatrix[8] + objectMatrix[9] * objectMatrix[9] + objectMatrix[10] * objectMatrix[10]);
        
        // Update position and scale
        m_SelectedObject->SetPosition(translation);
        m_SelectedObject->SetScale(scale);
        
        // For rotation, extract Euler angles from the rotation matrix
        if (operation == ImGuizmo::ROTATE || operation == ImGuizmo::TRANSLATE) {
            // Extract rotation from the upper-left 3x3 matrix
            float sy = std::sqrt(objectMatrix[0] * objectMatrix[0] + objectMatrix[4] * objectMatrix[4]);
            bool singular = sy < 1e-6;
            
            if (!singular) {
                rotation.x = std::atan2(objectMatrix[9], objectMatrix[10]) * 180.0f / 3.14159f;
                rotation.y = std::atan2(-objectMatrix[8], sy) * 180.0f / 3.14159f;
                rotation.z = std::atan2(objectMatrix[4], objectMatrix[0]) * 180.0f / 3.14159f;
            } else {
                rotation.x = std::atan2(-objectMatrix[6], objectMatrix[5]) * 180.0f / 3.14159f;
                rotation.y = std::atan2(-objectMatrix[8], sy) * 180.0f / 3.14159f;
                rotation.z = 0.0f;
            }
            
            m_SelectedObject->SetRotation(rotation);
        }
    }
}

Math::Vector3 SceneViewport::ScreenToWorldRay(float screenX, float screenY) {
    // Convert screen coordinates to world ray
    // This is a placeholder - full implementation would unproject screen coordinates
    return Math::Vector3(0.0f, 0.0f, 0.0f);
}

} // namespace LGE

