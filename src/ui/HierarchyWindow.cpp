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

#include "LGE/ui/HierarchyWindow.h"
#include "LGE/core/scene/World.h"
#include "LGE/core/scene/GameObject.h"
#include "LGE/core/scene/components/Transform.h"
#include "LGE/core/scene/components/MeshRenderer.h"
#include "LGE/core/scene/components/CameraComponent.h"
#include "imgui.h"

namespace LGE {

HierarchyWindow::HierarchyWindow(World* world)
    : m_World(world)
{
}

void HierarchyWindow::Render() {
    if (!m_World) return;
    
    ImGui::Begin("Hierarchy");
    
    // Create menu
    if (ImGui::Button("Create")) {
        ImGui::OpenPopup("CreateMenu");
    }
    
    if (ImGui::BeginPopup("CreateMenu")) {
        if (ImGui::MenuItem("Empty GameObject")) {
            if (m_World) {
                m_World->CreateGameObject("GameObject");
            }
        }
        
        if (ImGui::BeginMenu("3D Object")) {
            if (ImGui::MenuItem("Cube")) {
                if (m_World) {
                    auto go = m_World->CreateGameObject("Cube");
                    // Note: MeshRenderer and PrimitiveMesh would be implemented here
                    // go->AddComponent<MeshRenderer>()->SetMesh(PrimitiveMesh::CreateCube());
                }
            }
            if (ImGui::MenuItem("Sphere")) {
                if (m_World) {
                    auto go = m_World->CreateGameObject("Sphere");
                    // go->AddComponent<MeshRenderer>()->SetMesh(PrimitiveMesh::CreateSphere());
                }
            }
            if (ImGui::MenuItem("Plane")) {
                if (m_World) {
                    auto go = m_World->CreateGameObject("Plane");
                    // go->AddComponent<MeshRenderer>()->SetMesh(PrimitiveMesh::CreatePlane());
                }
            }
            ImGui::EndMenu();
        }
        
        
        if (ImGui::MenuItem("Camera")) {
            if (m_World) {
                m_World->CreateGameObject("Camera")->AddComponent<CameraComponent>();
            }
        }
        
        ImGui::EndPopup();
    }
    
    ImGui::Separator();
    
    // Render all root objects (no parent)
    auto allObjects = m_World->GetAllGameObjects();
    for (auto& go : allObjects) {
        if (!go->GetParent()) {
            RenderGameObjectNode(go);
        }
    }
    
    ImGui::End();
}

void HierarchyWindow::RenderGameObjectNode(std::shared_ptr<GameObject> gameObject) {
    if (!gameObject) return;
    
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    
    auto selected = m_SelectedGameObject.lock();
    if (selected == gameObject) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    
    if (gameObject->GetChildCount() == 0) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    
    // Use GUID as unique ID for tree node
    std::string nodeLabel = gameObject->GetName();
    if (!gameObject->IsActive()) {
        nodeLabel += " [Inactive]";
    }
    
    bool nodeOpen = ImGui::TreeNodeEx(gameObject.get(), flags, "%s", nodeLabel.c_str());
    
    // Selection
    if (ImGui::IsItemClicked()) {
        m_SelectedGameObject = gameObject;
    }
    
    // Context menu
    if (ImGui::BeginPopupContextItem()) {
        ShowContextMenu(gameObject);
        ImGui::EndPopup();
    }
    
    // Drag-drop for reparenting
    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("GAME_OBJECT", &gameObject, sizeof(std::shared_ptr<GameObject>));
        ImGui::Text("%s", gameObject->GetName().c_str());
        ImGui::EndDragDropSource();
    }
    
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAME_OBJECT")) {
            auto* droppedGO = static_cast<std::shared_ptr<GameObject>*>(payload->Data);
            if (*droppedGO && *droppedGO != gameObject) {
                (*droppedGO)->SetParent(gameObject);
            }
        }
        ImGui::EndDragDropTarget();
    }
    
    // Render children
    if (nodeOpen) {
        for (size_t i = 0; i < gameObject->GetChildCount(); ++i) {
            auto child = gameObject->GetChild(i);
            if (child) {
                RenderGameObjectNode(child);
            }
        }
        ImGui::TreePop();
    }
}

void HierarchyWindow::ShowContextMenu(std::shared_ptr<GameObject> gameObject) {
    if (!gameObject || !m_World) return;
    
    if (ImGui::MenuItem("Duplicate")) {
        // TODO: Implement GameObject duplication/instantiation
        // m_World->Instantiate(gameObject);
    }
    
    if (ImGui::MenuItem("Delete")) {
        m_World->DestroyGameObject(gameObject);
        // Clear selection if deleted object was selected
        if (m_SelectedGameObject.lock() == gameObject) {
            m_SelectedGameObject.reset();
        }
    }
    
    ImGui::Separator();
    
    if (ImGui::MenuItem("Create Empty Child")) {
        auto child = m_World->CreateGameObject("GameObject");
        child->SetParent(gameObject);
    }
}

std::shared_ptr<GameObject> HierarchyWindow::GetSelectedGameObject() const {
    return m_SelectedGameObject.lock();
}

void HierarchyWindow::SetSelectedGameObject(std::shared_ptr<GameObject> gameObject) {
    m_SelectedGameObject = gameObject;
}

} // namespace LGE

