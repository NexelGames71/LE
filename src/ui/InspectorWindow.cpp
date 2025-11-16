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

#include "LGE/ui/InspectorWindow.h"
#include "LGE/core/scene/GameObject.h"
#include "LGE/core/scene/components/Transform.h"
#include "LGE/core/scene/components/MeshRenderer.h"
#include "LGE/core/scene/components/CameraComponent.h"
#include "LGE/core/scene/components/LightComponent.h"
#include "LGE/core/scene/components/SkyLightComponent.h"
#include "LGE/core/scene/components/FogComponent.h"
#include "LGE/rendering/Lighting.h"
#include "LGE/core/scene/components/Rigidbody.h"
#include "LGE/core/scene/components/BoxCollider.h"
#include "LGE/core/scene/components/SphereCollider.h"
#include "LGE/core/scene/components/CapsuleCollider.h"
#include "LGE/core/scene/ComponentFactory.h"
#include "LGE/math/Vector.h"
#include "imgui.h"
#include <cstring>

namespace LGE {

InspectorWindow::InspectorWindow() {
}

void InspectorWindow::SetTarget(std::shared_ptr<GameObject> gameObject) {
    m_TargetGameObject = gameObject;
}

void InspectorWindow::Render() {
    ImGui::Begin("Inspector");
    
    auto target = m_TargetGameObject.lock();
    if (!target) {
        ImGui::Text("No object selected");
        ImGui::End();
        return;
    }
    
    // GameObject properties
    ImGui::PushID("GameObject");
    RenderGameObjectProperties(target);
    ImGui::PopID();
    
    ImGui::Separator();
    
    // Transform (always present)
    Transform* transform = target->GetTransform();
    if (transform) {
        RenderTransform(transform);
        ImGui::Separator();
    }
    
    // Other components
    const auto& components = target->GetAllComponents();
    for (const auto& [typeIndex, component] : components) {
        if (component) {
            RenderComponent(component.get());
            ImGui::Separator();
        }
    }
    
    // Add component button
    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("AddComponent");
    }
    
    if (ImGui::BeginPopup("AddComponent")) {
        auto types = ComponentFactory::GetRegisteredTypes();
        for (const auto& type : types) {
            // Skip Transform (always present) and components already added
            if (type == "Transform") continue;
            
            bool alreadyHas = false;
            for (const auto& [typeIdx, comp] : components) {
                if (comp && std::string(comp->GetTypeName()) == type) {
                    alreadyHas = true;
                    break;
                }
            }
            
            if (!alreadyHas && ImGui::MenuItem(type.c_str())) {
                // Use template-based AddComponent - need to handle each type
                // For now, just log that component creation needs template specialization
                // In a full implementation, you'd use a factory pattern or template specialization
                ImGui::Text("Component creation via factory not yet implemented");
            }
        }
        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void InspectorWindow::RenderGameObjectProperties(std::shared_ptr<GameObject> gameObject) {
    // Name
    char nameBuf[256];
    strncpy_s(nameBuf, gameObject->GetName().c_str(), 255);
    nameBuf[255] = '\0';
    if (ImGui::InputText("Name", nameBuf, 256)) {
        gameObject->SetName(nameBuf);
    }
    
    // Active
    bool active = gameObject->IsActive();
    if (ImGui::Checkbox("Active", &active)) {
        gameObject->SetActive(active);
    }
    
    // Tag
    char tagBuf[64];
    strncpy_s(tagBuf, gameObject->GetTag().c_str(), 63);
    tagBuf[63] = '\0';
    if (ImGui::InputText("Tag", tagBuf, 64)) {
        gameObject->SetTag(tagBuf);
    }
    
    // Layer
    int layer = static_cast<int>(gameObject->GetLayer());
    if (ImGui::InputInt("Layer", &layer)) {
        gameObject->SetLayer(static_cast<uint32_t>(layer));
    }
    
    // Static
    bool isStatic = gameObject->IsStatic();
    if (ImGui::Checkbox("Static", &isStatic)) {
        gameObject->SetStatic(isStatic);
    }
}

void InspectorWindow::RenderTransform(Transform* transform) {
    if (!transform) return;
    
    ImGui::PushID("Transform");
    
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Position
        Math::Vector3 pos = transform->GetPosition();
        float posArray[3] = { pos.x, pos.y, pos.z };
        if (ImGui::DragFloat3("Position", posArray, 0.1f)) {
            transform->SetPosition(Math::Vector3(posArray[0], posArray[1], posArray[2]));
        }
        
        // Rotation (Euler angles)
        Math::Vector3 rot = transform->GetRotation();
        float rotArray[3] = { rot.x, rot.y, rot.z };
        if (ImGui::DragFloat3("Rotation", rotArray, 1.0f)) {
            transform->SetRotation(Math::Vector3(rotArray[0], rotArray[1], rotArray[2]));
        }
        
        // Scale
        Math::Vector3 scale = transform->GetScale();
        float scaleArray[3] = { scale.x, scale.y, scale.z };
        if (ImGui::DragFloat3("Scale", scaleArray, 0.1f)) {
            transform->SetScale(Math::Vector3(scaleArray[0], scaleArray[1], scaleArray[2]));
        }
    }
    
    ImGui::PopID();
}

void InspectorWindow::RenderComponent(Component* component) {
    if (!component) return;
    
    std::string typeName = component->GetTypeName();
    ImGui::PushID(component);
    
    bool open = ImGui::CollapsingHeader(typeName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
    
    // Enable checkbox
    ImGui::SameLine();
    bool enabled = component->IsEnabled();
    if (ImGui::Checkbox("##Enabled", &enabled)) {
        component->SetEnabled(enabled);
    }
    
    if (open) {
        // Type-specific rendering
        if (typeName == "MeshRenderer") {
            RenderMeshRenderer(static_cast<MeshRenderer*>(component));
        } else if (typeName == "CameraComponent") {
            RenderCamera(static_cast<CameraComponent*>(component));
        } else if (typeName == "LightComponent") {
            RenderLightComponent(static_cast<LightComponent*>(component));
        } else if (typeName == "SkyLightComponent") {
            RenderSkyLightComponent(static_cast<SkyLightComponent*>(component));
        } else if (typeName == "FogComponent") {
            RenderFogComponent(static_cast<FogComponent*>(component));
        } else if (typeName == "Rigidbody") {
            RenderRigidbody(static_cast<Rigidbody*>(component));
        } else if (typeName == "BoxCollider") {
            RenderBoxCollider(static_cast<BoxCollider*>(component));
        } else if (typeName == "SphereCollider") {
            RenderSphereCollider(static_cast<SphereCollider*>(component));
        } else if (typeName == "CapsuleCollider") {
            RenderCapsuleCollider(static_cast<CapsuleCollider*>(component));
        }
    }
    
    ImGui::PopID();
}

void InspectorWindow::RenderMeshRenderer(MeshRenderer* renderer) {
    if (!renderer) return;
    
    ImGui::Text("Mesh: %s", renderer->GetMesh() ? "Assigned" : "None");
    
    size_t materialCount = renderer->GetMaterialCount();
    ImGui::Text("Materials: %zu", materialCount);
    
    bool castShadows = renderer->GetCastShadows();
    if (ImGui::Checkbox("Cast Shadows", &castShadows)) {
        renderer->SetCastShadows(castShadows);
    }
    
    bool receiveShadows = renderer->GetReceiveShadows();
    if (ImGui::Checkbox("Receive Shadows", &receiveShadows)) {
        renderer->SetReceiveShadows(receiveShadows);
    }
}

void InspectorWindow::RenderCamera(CameraComponent* camera) {
    if (!camera) return;
    
    // Projection type
    const char* projTypes[] = { "Perspective", "Orthographic" };
    int projType = static_cast<int>(camera->GetProjectionType());
    if (ImGui::Combo("Projection", &projType, projTypes, 2)) {
        camera->SetProjectionType(static_cast<CameraComponent::ProjectionType>(projType));
    }
    
    if (camera->GetProjectionType() == CameraComponent::ProjectionType::Perspective) {
        float fov = camera->GetFieldOfView();
        if (ImGui::SliderFloat("Field of View", &fov, 1.0f, 179.0f)) {
            camera->SetFieldOfView(fov);
        }
    } else {
        float size = camera->GetOrthographicSize();
        if (ImGui::DragFloat("Size", &size, 0.1f)) {
            camera->SetOrthographicSize(size);
        }
    }
    
    // Clipping planes
    float nearClip = camera->GetNearClipPlane();
    if (ImGui::DragFloat("Near Clip", &nearClip, 0.01f)) {
        camera->SetNearClipPlane(nearClip);
    }
    
    float farClip = camera->GetFarClipPlane();
    if (ImGui::DragFloat("Far Clip", &farClip, 1.0f)) {
        camera->SetFarClipPlane(farClip);
    }
    
    // Clear color
    Math::Vector4 clearColor = camera->GetClearColor();
    float colorArray[4] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
    if (ImGui::ColorEdit4("Clear Color", colorArray)) {
        camera->SetClearColor(Math::Vector4(colorArray[0], colorArray[1], colorArray[2], colorArray[3]));
    }
}

void InspectorWindow::RenderLightComponent(LightComponent* light) {
    if (!light) return;
    
    // Type dropdown
    const char* lightTypes[] = { "Directional", "Point", "Spot" };
    int currentType = static_cast<int>(light->Type);
    if (ImGui::Combo("Type", &currentType, lightTypes, 3)) {
        light->Type = static_cast<LightType>(currentType);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Directional: Sun/moon style, infinite distance\nPoint: Emits in all directions\nSpot: Flashlight/stage light");
        ImGui::EndTooltip();
    }
    
    // Mobility dropdown
    const char* mobilityTypes[] = { "Static", "Stationary", "Movable" };
    int currentMobility = static_cast<int>(light->Mobility);
    if (ImGui::Combo("Mobility", &currentMobility, mobilityTypes, 3)) {
        light->Mobility = static_cast<LightMobility>(currentMobility);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Static: Pre-baked, cannot move or change at runtime\nStationary: Can change intensity/color but not position\nMovable: Fully dynamic, can move and change at runtime");
        ImGui::EndTooltip();
    }
    
    // Color picker - CRITICAL: Edit directly on the component
    float color[3] = { light->Color.x, light->Color.y, light->Color.z };
    if (ImGui::ColorEdit3("Color", color)) {
        // Directly assign to component - LightSystem will pick this up next frame
        light->Color = Math::Vector3(color[0], color[1], color[2]);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Light tint (warm sunset = orangish, moon = blueish)");
        ImGui::EndTooltip();
    }
    
    // Intensity - CRITICAL: Edit directly on the component
    // Use DragFloat with direct pointer to component member
    if (ImGui::DragFloat("Intensity", &light->Intensity, 0.1f, 0.0f, 10000.0f)) {
        // Intensity is edited directly via pointer, no assignment needed
        // The LightSystem will pick this up on the next frame via CollectLights
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("How bright. Higher = more powerful");
        ImGui::EndTooltip();
    }
    
    // Range (for Point/Spot) - CRITICAL: Edit directly on the component (light is a pointer, so this is correct)
    if (light->IsPoint() || light->IsSpot()) {
        // Directly edit the component's Range property
        if (ImGui::DragFloat("Range", &light->Range, 0.1f, 0.1f, 1000.0f)) {
            // Range is edited directly, no assignment needed
            // The LightSystem will pick this up on the next frame via CollectLights
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("How far the light reaches; also used for performance culling");
            ImGui::EndTooltip();
        }
    }
    
    // Spot light angles - CRITICAL: Edit directly on the component
    if (light->IsSpot()) {
        // Convert to degrees for display
        float innerAngleDeg = light->InnerAngle * 180.0f / 3.14159f;
        if (ImGui::DragFloat("Inner Angle", &innerAngleDeg, 1.0f, 0.0f, 180.0f)) {
            // Convert back to radians and assign directly
            light->InnerAngle = innerAngleDeg * 3.14159f / 180.0f;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Full brightness cone angle (degrees)");
            ImGui::EndTooltip();
        }
        
        float outerAngleDeg = light->OuterAngle * 180.0f / 3.14159f;
        if (ImGui::DragFloat("Outer Angle", &outerAngleDeg, 1.0f, 0.0f, 180.0f)) {
            // Convert back to radians and assign directly
            light->OuterAngle = outerAngleDeg * 3.14159f / 180.0f;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Where it fully fades to black (degrees)");
            ImGui::EndTooltip();
        }
    }
    
    // Cast Shadows
    bool castShadows = light->CastShadows;
    if (ImGui::Checkbox("Cast Shadows", &castShadows)) {
        light->CastShadows = castShadows;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Whether this light will render to a shadow map");
        ImGui::EndTooltip();
    }
    
    // Advanced shadow settings (foldout)
    if (ImGui::TreeNode("Shadow Settings")) {
        float shadowBias = light->ShadowBias;
        if (ImGui::DragFloat("Shadow Bias", &shadowBias, 0.0001f, 0.0f, 0.1f, "%.4f")) {
            light->ShadowBias = shadowBias;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Fix z-fighting artifacts in shadows");
            ImGui::EndTooltip();
        }
        
        float shadowNormalBias = light->ShadowNormalBias;
        if (ImGui::DragFloat("Shadow Normal Bias", &shadowNormalBias, 0.01f, 0.0f, 1.0f)) {
            light->ShadowNormalBias = shadowNormalBias;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Additional bias along surface normal");
            ImGui::EndTooltip();
        }
        
        ImGui::TreePop();
    }
}

void InspectorWindow::RenderSkyLightComponent(SkyLightComponent* skyLight) {
    if (!skyLight) return;
    
    bool enabled = skyLight->Enabled;
    if (ImGui::Checkbox("Enabled", &enabled)) {
        skyLight->Enabled = enabled;
    }
    
    // Environment map path
    char pathBuffer[512];
    strncpy_s(pathBuffer, skyLight->EnvironmentMapPath.c_str(), sizeof(pathBuffer) - 1);
    pathBuffer[sizeof(pathBuffer) - 1] = '\0';
    if (ImGui::InputText("Environment Map", pathBuffer, sizeof(pathBuffer))) {
        skyLight->EnvironmentMapPath = std::string(pathBuffer);
    }
    ImGui::SameLine();
    if (ImGui::Button("...")) {
        // TODO: Open file browser for HDR/EXR cubemap
    }
    
    float intensity = skyLight->Intensity;
    if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 10.0f)) {
        skyLight->Intensity = intensity;
    }
    
    bool useDiffuseIBL = skyLight->UseDiffuseIBL;
    if (ImGui::Checkbox("Use Diffuse IBL", &useDiffuseIBL)) {
        skyLight->UseDiffuseIBL = useDiffuseIBL;
    }
    
    bool useSpecularIBL = skyLight->UseSpecularIBL;
    if (ImGui::Checkbox("Use Specular IBL", &useSpecularIBL)) {
        skyLight->UseSpecularIBL = useSpecularIBL;
    }
}

void InspectorWindow::RenderFogComponent(FogComponent* fog) {
    if (!fog) return;
    
    bool enabled = fog->Enabled;
    if (ImGui::Checkbox("Enabled", &enabled)) {
        fog->Enabled = enabled;
    }
    
    // Fog type
    const char* fogTypes[] = { "None", "Linear", "Exponential", "Exponential Squared", "Volumetric" };
    int currentType = static_cast<int>(fog->Type);
    if (ImGui::Combo("Fog Type", &currentType, fogTypes, 5)) {
        fog->Type = static_cast<FogType>(currentType);
    }
    
    if (fog->Type != FogType::None) {
        // Color
        float color[3] = { fog->Color.x, fog->Color.y, fog->Color.z };
        if (ImGui::ColorEdit3("Color", color)) {
            fog->Color = Math::Vector3(color[0], color[1], color[2]);
        }
        
        if (fog->Type == FogType::Linear) {
            float start = fog->StartDistance;
            if (ImGui::DragFloat("Start Distance", &start, 1.0f, 0.0f, 1000.0f)) {
                fog->StartDistance = start;
            }
            
            float end = fog->EndDistance;
            if (ImGui::DragFloat("End Distance", &end, 1.0f, 0.0f, 1000.0f)) {
                fog->EndDistance = end;
            }
        } else if (fog->Type == FogType::Exponential || fog->Type == FogType::ExponentialSquared) {
            float density = fog->Density;
            if (ImGui::DragFloat("Density", &density, 0.001f, 0.0f, 1.0f)) {
                fog->Density = density;
            }
        }
        
        // Height fog (future)
        if (ImGui::TreeNode("Height Fog (Future)")) {
            float height = fog->Height;
            if (ImGui::DragFloat("Height", &height, 1.0f, -100.0f, 100.0f)) {
                fog->Height = height;
            }
            
            float falloff = fog->HeightFalloff;
            if (ImGui::DragFloat("Height Falloff", &falloff, 0.1f, 0.0f, 10.0f)) {
                fog->HeightFalloff = falloff;
            }
            
            ImGui::TreePop();
        }
        
        // Volumetric fog (future)
        if (ImGui::TreeNode("Volumetric Fog (Future)")) {
            bool enableVolumetric = fog->EnableVolumetric;
            if (ImGui::Checkbox("Enable Volumetric", &enableVolumetric)) {
                fog->EnableVolumetric = enableVolumetric;
            }
            
            if (enableVolumetric) {
                float scattering = fog->VolumetricScattering;
                if (ImGui::DragFloat("Scattering", &scattering, 0.01f, 0.0f, 1.0f)) {
                    fog->VolumetricScattering = scattering;
                }
                
                float extinction = fog->VolumetricExtinction;
                if (ImGui::DragFloat("Extinction", &extinction, 0.01f, 0.0f, 1.0f)) {
                    fog->VolumetricExtinction = extinction;
                }
            }
            
            bool enableLightShafts = fog->EnableLightShafts;
            if (ImGui::Checkbox("Enable Light Shafts", &enableLightShafts)) {
                fog->EnableLightShafts = enableLightShafts;
            }
            
            ImGui::TreePop();
        }
    }
}

void InspectorWindow::RenderRigidbody(Rigidbody* rb) {
    if (!rb) return;
    
    // Mass
    float mass = rb->GetMass();
    if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.01f, 1000.0f)) {
        rb->SetMass(mass);
    }
    
    // Drag
    float drag = rb->GetDrag();
    if (ImGui::DragFloat("Drag", &drag, 0.01f, 0.0f, 10.0f)) {
        rb->SetDrag(drag);
    }
    
    // Angular drag
    float angularDrag = rb->GetAngularDrag();
    if (ImGui::DragFloat("Angular Drag", &angularDrag, 0.01f, 0.0f, 10.0f)) {
        rb->SetAngularDrag(angularDrag);
    }
    
    // Use gravity
    bool useGravity = rb->GetUseGravity();
    if (ImGui::Checkbox("Use Gravity", &useGravity)) {
        rb->SetUseGravity(useGravity);
    }
    
    // Constraints
    if (ImGui::TreeNode("Constraints")) {
        bool freezeX = rb->GetFreezePositionX();
        bool freezeY = rb->GetFreezePositionY();
        bool freezeZ = rb->GetFreezePositionZ();
        
        if (ImGui::Checkbox("Freeze Position X", &freezeX) ||
            ImGui::Checkbox("Freeze Position Y", &freezeY) ||
            ImGui::Checkbox("Freeze Position Z", &freezeZ)) {
            rb->SetFreezePosition(freezeX, freezeY, freezeZ);
        }
        
        bool freezeRotX = rb->GetFreezeRotationX();
        bool freezeRotY = rb->GetFreezeRotationY();
        bool freezeRotZ = rb->GetFreezeRotationZ();
        
        if (ImGui::Checkbox("Freeze Rotation X", &freezeRotX) ||
            ImGui::Checkbox("Freeze Rotation Y", &freezeRotY) ||
            ImGui::Checkbox("Freeze Rotation Z", &freezeRotZ)) {
            rb->SetFreezeRotation(freezeRotX, freezeRotY, freezeRotZ);
        }
        
        ImGui::TreePop();
    }
}

void InspectorWindow::RenderBoxCollider(BoxCollider* collider) {
    if (!collider) return;
    
    bool isTrigger = collider->GetIsTrigger();
    if (ImGui::Checkbox("Is Trigger", &isTrigger)) {
        collider->SetIsTrigger(isTrigger);
    }
    
    Math::Vector3 size = collider->GetSize();
    float sizeArray[3] = { size.x, size.y, size.z };
    if (ImGui::DragFloat3("Size", sizeArray, 0.1f)) {
        collider->SetSize(Math::Vector3(sizeArray[0], sizeArray[1], sizeArray[2]));
    }
}

void InspectorWindow::RenderSphereCollider(SphereCollider* collider) {
    if (!collider) return;
    
    bool isTrigger = collider->GetIsTrigger();
    if (ImGui::Checkbox("Is Trigger", &isTrigger)) {
        collider->SetIsTrigger(isTrigger);
    }
    
    float radius = collider->GetRadius();
    if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.01f, 100.0f)) {
        collider->SetRadius(radius);
    }
}

void InspectorWindow::RenderCapsuleCollider(CapsuleCollider* collider) {
    if (!collider) return;
    
    bool isTrigger = collider->GetIsTrigger();
    if (ImGui::Checkbox("Is Trigger", &isTrigger)) {
        collider->SetIsTrigger(isTrigger);
    }
    
    float radius = collider->GetRadius();
    if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.01f, 100.0f)) {
        collider->SetRadius(radius);
    }
    
    float height = collider->GetHeight();
    if (ImGui::DragFloat("Height", &height, 0.1f, 0.01f, 100.0f)) {
        collider->SetHeight(height);
    }
    
    const char* directions[] = { "X", "Y", "Z" };
    int direction = collider->GetDirection();
    if (ImGui::Combo("Direction", &direction, directions, 3)) {
        collider->SetDirection(direction);
    }
}

} // namespace LGE

