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

#include "LGE/ui/Hierarchy.h"
#include "LGE/core/scene/GameObject.h"
#include "LGE/rendering/Texture.h"
#include "imgui.h"

namespace LGE {

Hierarchy::Hierarchy()
    : m_SelectedObject(nullptr)
    , m_IconsLoaded(false)
    , m_OnCreateGameObject(nullptr)
{
}

Hierarchy::~Hierarchy() {
}

void Hierarchy::LoadIcons() {
    if (m_IconsLoaded) return;
    
    // Load plus icon
    m_PlusIcon = std::make_shared<Texture>();
    std::vector<std::string> plusPaths = {
        "assets/icons/PlusSymbol_12x.png",
        "unreal-engine-editor-icons/imgs/Icons/PlusSymbol_12x.png"
    };
    bool plusLoaded = false;
    for (const auto& path : plusPaths) {
        if (m_PlusIcon->LoadImageFile(path)) {
            plusLoaded = true;
            break;
        }
    }
    if (!plusLoaded) {
        m_PlusIcon.reset();
    }
    
    // Load search icon
    m_SearchIcon = std::make_shared<Texture>();
    std::vector<std::string> searchPaths = {
        "assets/icons/SearchGlass.png",
        "unreal-engine-editor-icons/imgs/Common/SearchGlass.png"
    };
    bool searchLoaded = false;
    for (const auto& path : searchPaths) {
        if (m_SearchIcon->LoadImageFile(path)) {
            searchLoaded = true;
            break;
        }
    }
    if (!searchLoaded) {
        m_SearchIcon.reset();
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

void Hierarchy::OnUIRender() {
    ImGui::Begin("Outliner", nullptr);
    
    // Load icons on first render
    if (!m_IconsLoaded) {
        LoadIcons();
    }
    
    // Plus button and search bar in same line
    ImGui::BeginGroup();
    
    // Plus button on the left
    ImVec2 buttonSize(20.0f, 20.0f);
    if (m_PlusIcon && m_PlusIcon->GetRendererID() != 0) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        
        if (ImGui::Button("##PlusButton", buttonSize)) {
            ImGui::OpenPopup("AddGameObjectMenu");
        }
        
        // Popup menu for adding GameObjects
        if (ImGui::BeginPopup("AddGameObjectMenu")) {
            if (ImGui::MenuItem("Empty GameObject")) {
                if (m_OnCreateGameObject) {
                    m_OnCreateGameObject("Empty");
                }
            }
            
            ImGui::Separator();
            
            // 3D Object submenu
            if (ImGui::BeginMenu("3D Object")) {
                if (ImGui::MenuItem("Cube")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("Cube");
                    }
                }
                if (ImGui::MenuItem("Sphere")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("Sphere");
                    }
                }
                if (ImGui::MenuItem("Plane")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("Plane");
                    }
                }
                if (ImGui::MenuItem("Cylinder")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("Cylinder");
                    }
                }
                if (ImGui::MenuItem("Capsule")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("Capsule");
                    }
                }
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            // Light submenu
            if (ImGui::BeginMenu("Light")) {
                if (ImGui::MenuItem("Directional Light")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("DirectionalLight");
                    }
                }
                if (ImGui::MenuItem("Point Light")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("PointLight");
                    }
                }
                if (ImGui::MenuItem("Spot Light")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("SpotLight");
                    }
                }
                if (ImGui::MenuItem("Sky Light")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("SkyLight");
                    }
                }
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Camera")) {
                if (m_OnCreateGameObject) {
                    m_OnCreateGameObject("Camera");
                }
            }
            
            ImGui::EndPopup();
        }
        
        // Draw plus icon in the center of the button
        if (ImGui::IsItemVisible()) {
            ImVec2 pos = ImGui::GetItemRectMin();
            ImVec2 rectSize = ImGui::GetItemRectSize();
            ImVec2 center = ImVec2(pos.x + rectSize.x * 0.5f, pos.y + rectSize.y * 0.5f);
            float iconSize = std::min(rectSize.x, rectSize.y) * 0.7f;
            ImVec2 iconMin = ImVec2(center.x - iconSize * 0.5f, center.y - iconSize * 0.5f);
            ImVec2 iconMax = ImVec2(center.x + iconSize * 0.5f, center.y + iconSize * 0.5f);
            ImGui::GetWindowDrawList()->AddImage(
                reinterpret_cast<void*>(static_cast<intptr_t>(m_PlusIcon->GetRendererID())),
                iconMin, iconMax,
                ImVec2(0, 0), ImVec2(1, 1));
        }
        
        ImGui::PopStyleColor(3);
    } else {
        // Fallback: simple plus button
        if (ImGui::Button("+", buttonSize)) {
            ImGui::OpenPopup("AddGameObjectMenu");
        }
        
        // Popup menu for adding GameObjects (fallback)
        if (ImGui::BeginPopup("AddGameObjectMenu")) {
            if (ImGui::MenuItem("Empty GameObject")) {
                if (m_OnCreateGameObject) {
                    m_OnCreateGameObject("Empty");
                }
            }
            
            ImGui::Separator();
            
            // 3D Object submenu
            if (ImGui::BeginMenu("3D Object")) {
                if (ImGui::MenuItem("Cube")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("Cube");
                    }
                }
                if (ImGui::MenuItem("Sphere")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("Sphere");
                    }
                }
                if (ImGui::MenuItem("Plane")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("Plane");
                    }
                }
                if (ImGui::MenuItem("Cylinder")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("Cylinder");
                    }
                }
                if (ImGui::MenuItem("Capsule")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("Capsule");
                    }
                }
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            // Light submenu
            if (ImGui::BeginMenu("Light")) {
                if (ImGui::MenuItem("Directional Light")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("DirectionalLight");
                    }
                }
                if (ImGui::MenuItem("Point Light")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("PointLight");
                    }
                }
                if (ImGui::MenuItem("Spot Light")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("SpotLight");
                    }
                }
                if (ImGui::MenuItem("Sky Light")) {
                    if (m_OnCreateGameObject) {
                        m_OnCreateGameObject("SkyLight");
                    }
                }
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Camera")) {
                if (m_OnCreateGameObject) {
                    m_OnCreateGameObject("Camera");
                }
            }
            
            ImGui::EndPopup();
        }
    }
    
    ImGui::SameLine(0, 4.0f);
    
    // Search bar with icon and placeholder
    ImGui::PushItemWidth(-1); // Fill remaining space
    static char searchBuffer[256] = "";
    
    // Add left padding for search icon
    float iconSize = 14.0f;
    float iconPadding = 6.0f;
    float leftPadding = iconSize + iconPadding * 2.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(leftPadding, ImGui::GetStyle().FramePadding.y));
    
    // Use InputTextWithHint for placeholder text
    if (ImGui::InputTextWithHint("##Search", "Search Objects", searchBuffer, sizeof(searchBuffer))) {
        // Search filter applied
    }
    
    ImGui::PopStyleVar();
    
    // Draw search icon on the left side of the input field
    if (m_SearchIcon && m_SearchIcon->GetRendererID() != 0 && ImGui::IsItemVisible()) {
        ImVec2 inputPos = ImGui::GetItemRectMin();
        ImVec2 inputSize = ImGui::GetItemRectSize();
        ImVec2 iconMin = ImVec2(inputPos.x + iconPadding, inputPos.y + (inputSize.y - iconSize) * 0.5f);
        ImVec2 iconMax = ImVec2(iconMin.x + iconSize, iconMin.y + iconSize);
        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<void*>(static_cast<intptr_t>(m_SearchIcon->GetRendererID())),
            iconMin, iconMax,
            ImVec2(0, 0), ImVec2(1, 1));
    }
    
    ImGui::PopItemWidth();
    ImGui::EndGroup();
    
    ImGui::Separator();
    
    // Scene hierarchy tree
    if (ImGui::TreeNode("Scene")) {
        // Show all GameObjects
        for (size_t i = 0; i < m_GameObjects.size(); ++i) {
            auto& obj = m_GameObjects[i];
            if (obj) {
                // Use PushID with pointer address to ensure unique IDs
                ImGui::PushID(static_cast<int>(reinterpret_cast<intptr_t>(obj.get())));
                
                bool isSelected = (m_SelectedObject == obj.get());
                
                // Check if this is a light GameObject (Directional Light or Sky Light)
                bool isLight = (obj->GetName().find("Light") != std::string::npos);
                
                // Draw icon if available
                if (isLight && m_LightActorIcon && m_LightActorIcon->GetRendererID() != 0) {
                    ImGui::Image(
                        reinterpret_cast<void*>(static_cast<intptr_t>(m_LightActorIcon->GetRendererID())),
                        ImVec2(16.0f, 16.0f),
                        ImVec2(0, 0), ImVec2(1, 1)
                    );
                    ImGui::SameLine();
                }
                
                // Use unique label with index to avoid conflicts
                std::string label = obj->GetName() + "##" + std::to_string(i);
                if (ImGui::Selectable(label.c_str(), isSelected)) {
                    m_SelectedObject = obj.get();
                    obj->SetSelected(true);
                    // Deselect others
                    for (auto& other : m_GameObjects) {
                        if (other.get() != obj.get()) {
                            other->SetSelected(false);
                        }
                    }
                }
                
                ImGui::PopID();
            }
        }
        
        ImGui::TreePop();
    }
    
    ImGui::End();
}

} // namespace LGE

