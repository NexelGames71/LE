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

#include "LGE/ui/Inspector.h"
#include "LGE/core/GameObject.h"
#include "LGE/core/Component.h"
#include "LGE/core/components/LightPropertiesComponent.h"
#include "LGE/math/Vector.h"
#include "LGE/rendering/Texture.h"
#include "LGE/rendering/Luminite/LuminiteSubsystem.h"
#include "LGE/rendering/Luminite/LuminiteComponents.h"
#include "LGE/rendering/Luminite/ShadowSystem.h"
#include "imgui.h"
#include <string>
#include <cmath>

namespace LGE {

Inspector::Inspector()
    : m_SelectedObject(nullptr)
    , m_LuminiteSubsystem(nullptr)
    , m_ShadowSystem(nullptr)
    , m_IconsLoaded(false)
{
}

void Inspector::LoadIcons() {
    if (m_IconsLoaded) return;
    
    // Load add icon
    m_AddIcon = std::make_shared<Texture>();
    std::vector<std::string> addIconPaths = {
        "unreal-engine-editor-icons/imgs/Icons/icon_add_40x.png",
        "assets/icons/icon_add_40x.png"
    };
    bool addIconLoaded = false;
    for (const auto& path : addIconPaths) {
        if (m_AddIcon->LoadImageFile(path)) {
            addIconLoaded = true;
            break;
        }
    }
    if (!addIconLoaded) {
        m_AddIcon.reset();
    }
    
    m_IconsLoaded = true;
}

Inspector::~Inspector() {
}

void Inspector::OnUIRender() {
    ImGui::Begin("Details", nullptr);
    
    if (!m_SelectedObject) {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }
    
    // Layer dropdown in top right corner
    float windowWidth = ImGui::GetWindowWidth();
    float layerDropdownWidth = 120.0f;
    float layerDropdownX = windowWidth - layerDropdownWidth - ImGui::GetStyle().WindowPadding.x;
    
    ImGui::SetCursorPosX(layerDropdownX);
    ImGui::Text("Layer:");
    ImGui::SameLine();
    static int currentLayer = 0;
    const char* layers[] = { "Default", "UI", "Environment", "Player", "Enemy", "Pickup", "Trigger" };
    ImGui::SetNextItemWidth(layerDropdownWidth - 50.0f);
    if (ImGui::Combo("##Layer", &currentLayer, layers, IM_ARRAYSIZE(layers))) {
        // Layer changed
    }
    
    ImGui::Spacing();
    
    // Object name
    ImGui::Text("Name:");
    char nameBuffer[256];
    strncpy_s(nameBuffer, m_SelectedObject->GetName().c_str(), sizeof(nameBuffer) - 1);
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
    if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer))) {
        m_SelectedObject->SetName(std::string(nameBuffer));
    }
    
    ImGui::Separator();
    
    // Transform section
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Position
        Math::Vector3 pos = m_SelectedObject->GetPosition();
        float position[3] = { pos.x, pos.y, pos.z };
        ImGui::Text("Position");
        ImGui::PushItemWidth(-1);
        
        // X axis
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Red for X
        ImGui::Text("X");
        ImGui::PopStyleColor();
        ImGui::SameLine(20.0f);
        if (ImGui::DragFloat("##PositionX", &position[0], 0.1f)) {
            m_SelectedObject->SetPosition(Math::Vector3(position[0], position[1], position[2]));
        }
        
        // Y axis
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green for Y
        ImGui::Text("Y");
        ImGui::PopStyleColor();
        ImGui::SameLine(20.0f);
        if (ImGui::DragFloat("##PositionY", &position[1], 0.1f)) {
            m_SelectedObject->SetPosition(Math::Vector3(position[0], position[1], position[2]));
        }
        
        // Z axis
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.4f, 0.8f, 1.0f)); // Blue for Z
        ImGui::Text("Z");
        ImGui::PopStyleColor();
        ImGui::SameLine(20.0f);
        if (ImGui::DragFloat("##PositionZ", &position[2], 0.1f)) {
            m_SelectedObject->SetPosition(Math::Vector3(position[0], position[1], position[2]));
        }
        
        ImGui::PopItemWidth();
        ImGui::Spacing();
        
        // Rotation
        Math::Vector3 rot = m_SelectedObject->GetRotation();
        float rotation[3] = { rot.x, rot.y, rot.z };
        ImGui::Text("Rotation");
        ImGui::PushItemWidth(-1);
        
        // X axis
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Red for X
        ImGui::Text("X");
        ImGui::PopStyleColor();
        ImGui::SameLine(20.0f);
        bool rotationChanged = false;
        if (ImGui::DragFloat("##RotationX", &rotation[0], 1.0f)) {
            m_SelectedObject->SetRotation(Math::Vector3(rotation[0], rotation[1], rotation[2]));
            rotationChanged = true;
        }
        
        // Y axis
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green for Y
        ImGui::Text("Y");
        ImGui::PopStyleColor();
        ImGui::SameLine(20.0f);
        if (ImGui::DragFloat("##RotationY", &rotation[1], 1.0f)) {
            m_SelectedObject->SetRotation(Math::Vector3(rotation[0], rotation[1], rotation[2]));
            rotationChanged = true;
        }
        
        // Z axis
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.4f, 0.8f, 1.0f)); // Blue for Z
        ImGui::Text("Z");
        ImGui::PopStyleColor();
        ImGui::SameLine(20.0f);
        if (ImGui::DragFloat("##RotationZ", &rotation[2], 1.0f)) {
            m_SelectedObject->SetRotation(Math::Vector3(rotation[0], rotation[1], rotation[2]));
            rotationChanged = true;
        }
        
        // Update light if rotation changed and this is a light object
        if (rotationChanged && m_SelectedObject) {
            bool isLightObject = (m_SelectedObject->GetName().find("Light") != std::string::npos);
            if (isLightObject) {
                auto* lightProps = m_SelectedObject->GetComponent<LightPropertiesComponent>();
                if (lightProps) {
                    UpdateLightInLuminite(m_SelectedObject, lightProps);
                }
            }
        }
        
        ImGui::PopItemWidth();
        ImGui::Spacing();
        
        // Scale
        Math::Vector3 scl = m_SelectedObject->GetScale();
        float scale[3] = { scl.x, scl.y, scl.z };
        ImGui::Text("Scale");
        ImGui::PushItemWidth(-1);
        
        // X axis
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Red for X
        ImGui::Text("X");
        ImGui::PopStyleColor();
        ImGui::SameLine(20.0f);
        if (ImGui::DragFloat("##ScaleX", &scale[0], 0.1f)) {
            m_SelectedObject->SetScale(Math::Vector3(scale[0], scale[1], scale[2]));
        }
        
        // Y axis
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green for Y
        ImGui::Text("Y");
        ImGui::PopStyleColor();
        ImGui::SameLine(20.0f);
        if (ImGui::DragFloat("##ScaleY", &scale[1], 0.1f)) {
            m_SelectedObject->SetScale(Math::Vector3(scale[0], scale[1], scale[2]));
        }
        
        // Z axis
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.4f, 0.8f, 1.0f)); // Blue for Z
        ImGui::Text("Z");
        ImGui::PopStyleColor();
        ImGui::SameLine(20.0f);
        if (ImGui::DragFloat("##ScaleZ", &scale[2], 0.1f)) {
            m_SelectedObject->SetScale(Math::Vector3(scale[0], scale[1], scale[2]));
        }
        
        ImGui::PopItemWidth();
    }
    
    // Light Properties section (mandatory for light objects) - top level like Transform
    bool isLightObject = (m_SelectedObject->GetName().find("Light") != std::string::npos);
    if (isLightObject) {
        // Ensure LightPropertiesComponent exists (mandatory)
        auto* lightProps = m_SelectedObject->GetComponent<LightPropertiesComponent>();
        if (!lightProps) {
            lightProps = m_SelectedObject->AddComponent<LightPropertiesComponent>();
        }
        
        // Render Light Properties as a top-level collapsible header (like Transform)
        if (ImGui::CollapsingHeader("Light Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            RenderLightProperties(lightProps);
        }
    }
    
    // Add Component button
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Load icons on first render
    if (!m_IconsLoaded) {
        LoadIcons();
    }
    
    // Button with icon
    ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, 30.0f);
    if (m_AddIcon && m_AddIcon->GetRendererID() != 0) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        
        if (ImGui::Button("##AddComponent", buttonSize)) {
            ImGui::OpenPopup("AddComponentMenu");
        }
        
        // Popup menu for adding components
        if (ImGui::BeginPopup("AddComponentMenu")) {
            // Set popup width for better appearance
            ImGui::SetNextItemWidth(250.0f);
            
            // Header
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Add Component");
            ImGui::Separator();
            ImGui::Spacing();
            
            bool isLightObject = (m_SelectedObject->GetName().find("Light") != std::string::npos);
            
            // Lighting Components Section
            if (ImGui::BeginMenu("Lighting")) {
                if (!isLightObject) {
                    if (ImGui::MenuItem("Light Properties", nullptr, false, !m_SelectedObject->HasComponent<LightPropertiesComponent>())) {
                        if (m_SelectedObject && !m_SelectedObject->HasComponent<LightPropertiesComponent>()) {
                            m_SelectedObject->AddComponent<LightPropertiesComponent>();
                        }
                    }
                } else {
                    ImGui::TextDisabled("Light Properties (Already attached)");
                }
                
                // Add more lighting components here
                // if (ImGui::MenuItem("Point Light")) {
                //     m_SelectedObject->AddComponent<PointLightComponent>();
                // }
                
                ImGui::EndMenu();
            }
            
            // Rendering Components Section
            if (ImGui::BeginMenu("Rendering")) {
                ImGui::TextDisabled("No rendering components available");
                // Add rendering components here
                // if (ImGui::MenuItem("Mesh Renderer")) {
                //     m_SelectedObject->AddComponent<MeshRendererComponent>();
                // }
                // if (ImGui::MenuItem("Sprite Renderer")) {
                //     m_SelectedObject->AddComponent<SpriteRendererComponent>();
                // }
                ImGui::EndMenu();
            }
            
            // Physics Components Section
            if (ImGui::BeginMenu("Physics")) {
                ImGui::TextDisabled("No physics components available");
                // Add physics components here
                // if (ImGui::MenuItem("Rigidbody")) {
                //     m_SelectedObject->AddComponent<RigidbodyComponent>();
                // }
                // if (ImGui::MenuItem("Collider")) {
                //     m_SelectedObject->AddComponent<ColliderComponent>();
                // }
                ImGui::EndMenu();
            }
            
            // Audio Components Section
            if (ImGui::BeginMenu("Audio")) {
                ImGui::TextDisabled("No audio components available");
                // Add audio components here
                // if (ImGui::MenuItem("Audio Source")) {
                //     m_SelectedObject->AddComponent<AudioSourceComponent>();
                // }
                ImGui::EndMenu();
            }
            
            ImGui::EndPopup();
        }
        
        // Draw icon and text
        if (ImGui::IsItemVisible()) {
            ImVec2 pos = ImGui::GetItemRectMin();
            ImVec2 rectSize = ImGui::GetItemRectSize();
            
            // Draw icon
            float iconSize = 16.0f;
            ImVec2 iconMin = ImVec2(pos.x + 10.0f, pos.y + (rectSize.y - iconSize) * 0.5f);
            ImVec2 iconMax = ImVec2(iconMin.x + iconSize, iconMin.y + iconSize);
            ImGui::GetWindowDrawList()->AddImage(
                reinterpret_cast<void*>(static_cast<intptr_t>(m_AddIcon->GetRendererID())),
                iconMin, iconMax,
                ImVec2(0, 0), ImVec2(1, 1)
            );
            
            // Draw text
            ImVec2 textPos = ImVec2(iconMax.x + 8.0f, pos.y + (rectSize.y - ImGui::GetFontSize()) * 0.5f);
            ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), "Add Component");
        }
        
        ImGui::PopStyleColor(3);
    } else {
        // Fallback: simple button without icon
        if (ImGui::Button("+ Add Component", buttonSize)) {
            ImGui::OpenPopup("AddComponentMenu");
        }
        
        // Popup menu for adding components (same as above)
        if (ImGui::BeginPopup("AddComponentMenu")) {
            // Set popup width for better appearance
            ImGui::SetNextItemWidth(250.0f);
            
            // Header
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Add Component");
            ImGui::Separator();
            ImGui::Spacing();
            
            bool isLightObject = (m_SelectedObject->GetName().find("Light") != std::string::npos);
            
            // Lighting Components Section
            if (ImGui::BeginMenu("Lighting")) {
                if (!isLightObject) {
                    if (ImGui::MenuItem("Light Properties", nullptr, false, !m_SelectedObject->HasComponent<LightPropertiesComponent>())) {
                        if (m_SelectedObject && !m_SelectedObject->HasComponent<LightPropertiesComponent>()) {
                            m_SelectedObject->AddComponent<LightPropertiesComponent>();
                        }
                    }
                } else {
                    ImGui::TextDisabled("Light Properties (Already attached)");
                }
                ImGui::EndMenu();
            }
            
            // Rendering Components Section
            if (ImGui::BeginMenu("Rendering")) {
                ImGui::TextDisabled("No rendering components available");
                ImGui::EndMenu();
            }
            
            // Physics Components Section
            if (ImGui::BeginMenu("Physics")) {
                ImGui::TextDisabled("No physics components available");
                ImGui::EndMenu();
            }
            
            // Audio Components Section
            if (ImGui::BeginMenu("Audio")) {
                ImGui::TextDisabled("No audio components available");
                ImGui::EndMenu();
            }
            
            ImGui::EndPopup();
        }
    }
    
    ImGui::End();
}

void Inspector::RenderLightProperties(LightPropertiesComponent* lightProps) {
    if (!lightProps) return;
            // Color with wheel picker
            Math::Vector3 color = lightProps->GetColor();
            float colorArray[3] = { color.x, color.y, color.z };
            ImGui::Text("Color");
            
            // Show color button that opens the picker
            ImVec4 colorVec4(color.x, color.y, color.z, 1.0f);
            if (ImGui::ColorButton("##LightColorButton", colorVec4, ImGuiColorEditFlags_NoTooltip, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f))) {
                ImGui::OpenPopup("LightColorPicker");
            }
            
            // Color picker popup with wheel
            if (ImGui::BeginPopup("LightColorPicker")) {
                ImGui::Text("Select Color");
                ImGui::Separator();
                
                // Use ColorPicker3 with hue wheel flag for the wheel + triangle interface
                if (ImGui::ColorPicker3("##LightColorPicker", colorArray, 
                    ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs)) {
                    lightProps->SetColor(Math::Vector3(colorArray[0], colorArray[1], colorArray[2]));
                    UpdateLightInLuminite(m_SelectedObject, lightProps);
                }
                
                ImGui::EndPopup();
            }
            
            // Also show RGB inputs below for precise control
            ImGui::Spacing();
            ImGui::PushItemWidth(-1);
            ImGui::Text("RGB");
            if (ImGui::DragFloat3("##LightColorRGB", colorArray, 0.01f, 0.0f, 1.0f)) {
                lightProps->SetColor(Math::Vector3(colorArray[0], colorArray[1], colorArray[2]));
                UpdateLightInLuminite(m_SelectedObject, lightProps);
            }
            ImGui::PopItemWidth();
            
            // Intensity
            float intensity = lightProps->GetIntensity();
            ImGui::Text("Intensity");
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat("##LightIntensity", &intensity, 0.1f, 0.0f, 10.0f)) {
                lightProps->SetIntensity(intensity);
                UpdateLightInLuminite(m_SelectedObject, lightProps);
            }
            ImGui::PopItemWidth();
            
            // Temperature
            float temperature = lightProps->GetTemperature();
            ImGui::Text("Temperature (K)");
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat("##LightTemperature", &temperature, 100.0f, 1000.0f, 20000.0f)) {
                lightProps->SetTemperature(temperature);
                // Update color based on temperature
                Math::Vector3 tempColor = lightProps->GetColorFromTemperature();
                lightProps->SetColor(tempColor);
                UpdateLightInLuminite(m_SelectedObject, lightProps);
            }
            ImGui::PopItemWidth();
            
            // Indirect Intensity
            float indirectIntensity = lightProps->GetIndirectIntensity();
            ImGui::Text("Indirect Intensity");
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat("##LightIndirectIntensity", &indirectIntensity, 0.1f, 0.0f, 10.0f)) {
                lightProps->SetIndirectIntensity(indirectIntensity);
                UpdateLightInLuminite(m_SelectedObject, lightProps);
            }
            ImGui::PopItemWidth();
            
            // Cast Shadows
            bool castShadows = lightProps->GetCastShadows();
            if (ImGui::Checkbox("Cast Shadows", &castShadows)) {
                lightProps->SetCastShadows(castShadows);
                UpdateLightInLuminite(m_SelectedObject, lightProps);
                
                // Update shadow system registration
                if (m_ShadowSystem) {
                    if (castShadows) {
                        // Calculate direction from rotation
                        Math::Vector3 rotation = m_SelectedObject->GetRotation();
                        float rotX = rotation.x * 3.14159f / 180.0f;
                        float rotY = rotation.y * 3.14159f / 180.0f;
                        Math::Vector3 direction = Math::Vector3(
                            std::sin(rotY) * std::cos(rotX),
                            -std::sin(rotX),
                            std::cos(rotY) * std::cos(rotX)
                        );
                        m_ShadowSystem->RegisterDirectionalLightShadow(
                            m_SelectedObject,
                            direction,
                            m_SelectedObject->GetPosition(),
                            4  // 4 cascades
                        );
                    } else {
                        m_ShadowSystem->UnregisterLightShadow(m_SelectedObject);
                    }
                }
            }
            
            // Volumetric Scattering
            float volumetricScattering = lightProps->GetVolumetricScattering();
            ImGui::Text("Volumetric Scattering");
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat("##LightVolumetricScattering", &volumetricScattering, 0.1f, 0.0f, 10.0f)) {
                lightProps->SetVolumetricScattering(volumetricScattering);
                UpdateLightInLuminite(m_SelectedObject, lightProps);
            }
            ImGui::PopItemWidth();
            
            // Specular Contribution
            float specularContribution = lightProps->GetSpecularContribution();
            ImGui::Text("Specular Contribution");
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat("##LightSpecularContribution", &specularContribution, 0.1f, 0.0f, 10.0f)) {
                lightProps->SetSpecularContribution(specularContribution);
                UpdateLightInLuminite(m_SelectedObject, lightProps);
            }
            ImGui::PopItemWidth();
}

void Inspector::UpdateLightInLuminite(GameObject* obj, LightPropertiesComponent* lightProps) {
    if (!obj || !lightProps || !m_LuminiteSubsystem) return;
    
    // Check if this is a light object
    bool isLightObject = (obj->GetName().find("Light") != std::string::npos);
    if (!isLightObject) return;
    
    // Calculate direction from rotation
    Math::Vector3 rotation = obj->GetRotation();
    float rotX = rotation.x * 3.14159f / 180.0f;
    float rotY = rotation.y * 3.14159f / 180.0f;
    
    Math::Vector3 direction = Math::Vector3(
        std::sin(rotY) * std::cos(rotX),
        -std::sin(rotX),
        std::cos(rotY) * std::cos(rotX)
    );
    
    // Create Luminite directional light
    Luminite::DirectionalLight luminiteDirLight;
    luminiteDirLight.color = lightProps->GetColor();
    luminiteDirLight.intensity = lightProps->GetIntensity();
    luminiteDirLight.direction = direction;
    luminiteDirLight.castShadows = lightProps->GetCastShadows();
    luminiteDirLight.enabled = lightProps->IsEnabled();
    
    // Re-register the light (this will update it)
    m_LuminiteSubsystem->UnregisterLight(obj);
    m_LuminiteSubsystem->RegisterDirectionalLight(
        obj,
        luminiteDirLight,
        obj->GetPosition(),
        direction
    );
}

} // namespace LGE

