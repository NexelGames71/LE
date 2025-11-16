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

#include "LGE/core/scene/GameObject.h"
#include "LGE/core/scene/Component.h"
#include "LGE/core/scene/ComponentFactory.h"
#include "LGE/core/scene/components/Transform.h"
#include "LGE/core/Log.h"
#include "LGE/core/scene/World.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <sstream>
#include <algorithm>

namespace LGE {

uint32_t GameObject::s_NextID = 1;

GameObject::GameObject(const std::string& name)
    : m_GUID(GUID::Generate())
    , m_Name(name)
    , m_Tag("Untagged")
    , m_Layer(0)
    , m_IsActive(true)
    , m_IsStatic(false)
    , m_Selected(false)
    , m_IsDestroyed(false)
    , m_HasStarted(false)
    , m_ID(s_NextID++)
    , m_Position(0.0f, 0.0f, 0.0f)
    , m_Rotation(0.0f, 0.0f, 0.0f)
    , m_Scale(1.0f, 1.0f, 1.0f)
    , m_TransformDirty(true)
    , m_TransformComponent(nullptr)
    , m_World(nullptr)
{
    // Add Transform component by default
    auto transform = AddComponent<Transform>();
    m_TransformComponent = transform;  // Cache the pointer
}

GameObject::~GameObject() {
    // Cleanup components
    for (auto& [type, component] : m_Components) {
        if (component) {
            component->OnDestroy();
            component->OnDetach();
        }
    }
    m_Components.clear();
    
    // Remove from parent
    RemoveParent();
    
    // Clear children
    m_Children.clear();
}

void GameObject::SetActive(bool active) {
    if (m_IsActive == active) return;
    
    m_IsActive = active;
    NotifyComponentsActiveChanged(active);
    
    // Recursively affect children
    for (auto& child : m_Children) {
        if (child) {
            child->SetActive(active);
        }
    }
}

bool GameObject::IsActiveInHierarchy() const {
    if (!m_IsActive) return false;
    
    auto parent = m_Parent.lock();
    if (parent) {
        return parent->IsActiveInHierarchy();
    }
    
    return true;
}

void GameObject::NotifyComponentsActiveChanged(bool active) {
    for (auto& [type, component] : m_Components) {
        if (component) {
            if (active) {
                component->OnEnable();
            } else {
                component->OnDisable();
            }
        }
    }
}

void GameObject::SetParent(std::shared_ptr<GameObject> parent) {
    // Remove from old parent
    auto oldParent = m_Parent.lock();
    if (oldParent) {
        auto& siblings = oldParent->m_Children;
        siblings.erase(
            std::remove_if(siblings.begin(), siblings.end(),
                [this](const std::shared_ptr<GameObject>& child) {
                    return child.get() == this;
                }),
            siblings.end()
        );
    }
    
    // Set new parent
    m_Parent = parent;
    
    if (parent) {
        parent->m_Children.push_back(shared_from_this());
        
        // Update transform hierarchy if Transform component exists
        if (m_TransformComponent && parent->m_TransformComponent) {
            // Transform hierarchy is managed by Transform component
        }
    }
}

void GameObject::RemoveParent() {
    SetParent(nullptr);
}

std::shared_ptr<GameObject> GameObject::GetChild(size_t index) const {
    if (index < m_Children.size()) {
        return m_Children[index];
    }
    return nullptr;
}

std::shared_ptr<GameObject> GameObject::FindChild(const std::string& childName) const {
    for (const auto& child : m_Children) {
        if (child && child->m_Name == childName) {
            return child;
        }
    }
    return nullptr;
}

std::shared_ptr<GameObject> GameObject::FindChildRecursive(const std::string& childName) const {
    // Check direct children first
    auto child = FindChild(childName);
    if (child) return child;
    
    // Recursively search children
    for (const auto& c : m_Children) {
        if (c) {
            child = c->FindChildRecursive(childName);
            if (child) return child;
        }
    }
    
    return nullptr;
}

bool GameObject::RemoveComponent(Component* component) {
    if (!component) return false;
    
    for (auto it = m_Components.begin(); it != m_Components.end(); ++it) {
        if (it->second.get() == component) {
            // Clear Transform cache if removing Transform component
            if (component == m_TransformComponent) {
                m_TransformComponent = nullptr;
            }
            
            component->OnDestroy();
            component->OnDetach();
            
            m_Components.erase(it);
            return true;
        }
    }
    
    return false;
}

Component* GameObject::AddComponent(std::unique_ptr<Component> component) {
    if (!component) return nullptr;
    
    Component* ptr = component.get();
    component->OnAttach(this);
    
    // Get type_index from the actual component type
    std::type_index typeIndex(typeid(*ptr));
    m_Components[typeIndex] = std::move(component);
    
    // Cache Transform component
    if (typeIndex == typeid(Transform)) {
        m_TransformComponent = static_cast<Transform*>(ptr);
    }
    
    // Call Awake if game object has already started
    if (m_HasStarted && IsActiveInHierarchy()) {
        ptr->Awake();
        ptr->Start();
        ptr->SetHasStarted(true);
    }
    
    return ptr;
}

void GameObject::UpdateComponents(float deltaTime) {
    for (auto& [type, component] : m_Components) {
        if (component && component->IsEnabled()) {
            component->Update(deltaTime);
        }
    }
}

void GameObject::Awake() {
    if (m_HasStarted) return;
    
    // Awake all components
    for (auto& [type, component] : m_Components) {
        if (component && component->IsEnabled()) {
            component->Awake();
        }
    }
}

void GameObject::Start() {
    if (m_HasStarted) return;
    m_HasStarted = true;
    
    // Start all components
    for (auto& [type, component] : m_Components) {
        if (component && component->IsEnabled()) {
            component->Start();
            component->SetHasStarted(true);
        }
    }
    
    // Start children
    for (auto& child : m_Children) {
        if (child) {
            child->Start();
        }
    }
}

void GameObject::Update(float deltaTime) {
    if (!IsActiveInHierarchy()) return;
    
    // Update components
    for (auto& [type, component] : m_Components) {
        if (component && component->IsEnabled()) {
            component->Update(deltaTime);
        }
    }
    
    // Update children
    for (auto& child : m_Children) {
        if (child) {
            child->Update(deltaTime);
        }
    }
}

void GameObject::LateUpdate(float deltaTime) {
    if (!IsActiveInHierarchy()) return;
    
    // Late update components
    for (auto& [type, component] : m_Components) {
        if (component && component->IsEnabled()) {
            component->LateUpdate(deltaTime);
        }
    }
    
    // Late update children
    for (auto& child : m_Children) {
        if (child) {
            child->LateUpdate(deltaTime);
        }
    }
}

void GameObject::FixedUpdate(float fixedDeltaTime) {
    if (!IsActiveInHierarchy()) return;
    
    // Fixed update components
    for (auto& [type, component] : m_Components) {
        if (component && component->IsEnabled()) {
            component->FixedUpdate(fixedDeltaTime);
        }
    }
    
    // Fixed update children
    for (auto& child : m_Children) {
        if (child) {
            child->FixedUpdate(fixedDeltaTime);
        }
    }
}

void GameObject::OnDestroy() {
    // Notify all components
    for (auto& [type, component] : m_Components) {
        if (component) {
            component->OnDestroy();
        }
    }
    
    // Notify children
    for (auto& child : m_Children) {
        if (child) {
            child->OnDestroy();
        }
    }
}

void GameObject::Destroy() {
    if (m_IsDestroyed) return;
    
    m_IsDestroyed = true;
    
    // Destroy all children
    for (auto& child : m_Children) {
        if (child) {
            child->Destroy();
        }
    }
    
    OnDestroy();
    
    // Remove from parent
    RemoveParent();
    
    // Notify world (if World class exists)
    // if (m_World) {
    //     m_World->RemoveGameObject(shared_from_this());
    // }
}

// Legacy transform methods (for backward compatibility)
void GameObject::SetPosition(const Math::Vector3& position) {
    m_Position = position;
    m_TransformDirty = true;
    
    if (m_TransformComponent) {
        m_TransformComponent->SetPosition(position);
    }
}

void GameObject::SetRotation(const Math::Vector3& rotation) {
    m_Rotation = rotation;
    m_TransformDirty = true;
    
    if (m_TransformComponent) {
        m_TransformComponent->SetRotation(rotation);
    }
}

void GameObject::SetScale(const Math::Vector3& scale) {
    m_Scale = scale;
    m_TransformDirty = true;
    
    if (m_TransformComponent) {
        m_TransformComponent->SetScale(scale);
    }
}

Math::Vector3 GameObject::GetPosition() const {
    if (m_TransformComponent) {
        return m_TransformComponent->GetPosition();
    }
    return m_Position;
}

Math::Vector3 GameObject::GetRotation() const {
    if (m_TransformComponent) {
        return m_TransformComponent->GetRotation();
    }
    return m_Rotation;
}

Math::Vector3 GameObject::GetScale() const {
    if (m_TransformComponent) {
        return m_TransformComponent->GetScale();
    }
    return m_Scale;
}

const Math::Matrix4& GameObject::GetTransformMatrix() {
    if (m_TransformComponent) {
        return m_TransformComponent->GetLocalMatrix();
    }
    
    UpdateTransformMatrix();
    return m_TransformMatrix;
}

Transform* GameObject::GetTransform() const {
    return m_TransformComponent;
}

void GameObject::UpdateTransformMatrix() {
    if (!m_TransformDirty) return;
    
    // Build transform matrix: T * R * S
    Math::Matrix4 translation = Math::Matrix4::Translate(m_Position);
    
    // Rotation as Euler angles (in degrees)
    float rx = m_Rotation.x * 3.14159f / 180.0f;
    float ry = m_Rotation.y * 3.14159f / 180.0f;
    float rz = m_Rotation.z * 3.14159f / 180.0f;
    
    Math::Matrix4 rotX = Math::Matrix4::Rotate(rx, Math::Vector3(1.0f, 0.0f, 0.0f));
    Math::Matrix4 rotY = Math::Matrix4::Rotate(ry, Math::Vector3(0.0f, 1.0f, 0.0f));
    Math::Matrix4 rotZ = Math::Matrix4::Rotate(rz, Math::Vector3(0.0f, 0.0f, 1.0f));
    Math::Matrix4 rotation = rotZ * rotY * rotX;
    
    Math::Matrix4 scale = Math::Matrix4::Scale(m_Scale);
    
    m_TransformMatrix = translation * rotation * scale;
    m_TransformDirty = false;
}

std::string GameObject::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"guid\": \"" << m_GUID.ToString() << "\",\n";
    json << "  \"name\": \"" << m_Name << "\",\n";
    json << "  \"tag\": \"" << m_Tag << "\",\n";
    json << "  \"layer\": " << m_Layer << ",\n";
    json << "  \"isActive\": " << (m_IsActive ? "true" : "false") << ",\n";
    json << "  \"isStatic\": " << (m_IsStatic ? "true" : "false") << ",\n";
    
    // Serialize transform
    if (m_TransformComponent) {
        json << "  \"transform\": " << m_TransformComponent->Serialize() << ",\n";
    }
    
    // Serialize components
    json << "  \"components\": [\n";
    bool first = true;
    for (const auto& [type, component] : m_Components) {
        if (component && component->GetTypeName() != std::string("Transform")) {
            if (!first) json << ",\n";
            json << "    " << component->Serialize();
            first = false;
        }
    }
    json << "\n  ],\n";
    
    // Serialize children
    json << "  \"children\": [\n";
    first = true;
    for (const auto& child : m_Children) {
        if (child) {
            if (!first) json << ",\n";
            json << "    " << child->Serialize();
            first = false;
        }
    }
    json << "\n  ]\n";
    json << "}";
    
    return json.str();
}

std::shared_ptr<GameObject> GameObject::Deserialize(const std::string& json, World* world) {
    try {
        auto j = nlohmann::json::parse(json);
        
        // Create GameObject
        std::string name = j.value("name", "GameObject");
        auto gameObject = Create(name);
        gameObject->SetWorld(world);
        
        // Parse GUID
        if (j.contains("guid") && j["guid"].is_string()) {
            GUID guid = GUID::FromString(j["guid"]);
            if (guid.IsValid()) {
                // Note: GameObject GUID is set in constructor, but we can't change it
                // This is a limitation - in a full implementation, you'd want to set it during creation
            }
        }
        
        // Parse basic properties
        if (j.contains("tag") && j["tag"].is_string()) {
            gameObject->SetTag(j["tag"]);
        }
        if (j.contains("layer") && j["layer"].is_number()) {
            gameObject->SetLayer(j["layer"]);
        }
        if (j.contains("isActive") && j["isActive"].is_boolean()) {
            gameObject->SetActive(j["isActive"]);
        }
        if (j.contains("isStatic") && j["isStatic"].is_boolean()) {
            gameObject->SetStatic(j["isStatic"]);
        }
        
        // Parse transform
        if (j.contains("transform") && j["transform"].is_object()) {
            std::string transformJson = j["transform"].dump();
            auto* transform = gameObject->GetTransform();
            if (transform) {
                transform->Deserialize(transformJson);
            }
        }
        
        // Parse components
        if (j.contains("components") && j["components"].is_array()) {
            for (const auto& compJson : j["components"]) {
                if (compJson.is_object() && compJson.contains("type") && compJson["type"].is_string()) {
                    std::string typeName = compJson["type"];
                    
                    // Skip Transform as it's already handled
                    if (typeName == "Transform") {
                        continue;
                    }
                    
                    // Create component using factory
                    auto component = ComponentFactory::Create(typeName);
                    if (component) {
                        std::string compJsonStr = compJson.dump();
                        component->Deserialize(compJsonStr);
                        // Use the non-template AddComponent method
                        Component* addedComponent = gameObject->AddComponent(std::move(component));
                        (void)addedComponent; // Suppress unused variable warning
                    } else {
                        Log::Warn("GameObject::Deserialize: Unknown component type: " + typeName);
                    }
                }
            }
        }
        
        // Parse children (recursively)
        if (j.contains("children") && j["children"].is_array()) {
            for (const auto& childJson : j["children"]) {
                if (childJson.is_object()) {
                    std::string childJsonStr = childJson.dump();
                    auto child = Deserialize(childJsonStr, world);
                    if (child) {
                        child->SetParent(gameObject);
                        gameObject->m_Children.push_back(child);
                    }
                }
            }
        }
        
        return gameObject;
    } catch (const std::exception& e) {
        Log::Error("GameObject::Deserialize failed: " + std::string(e.what()));
        return nullptr;
    }
}

std::shared_ptr<GameObject> GameObject::Create(const std::string& name) {
    return std::make_shared<GameObject>(name);
}

std::shared_ptr<GameObject> GameObject::CreatePrimitive(const std::string& primitiveType) {
    auto go = Create(primitiveType);
    
    // Add mesh renderer and appropriate mesh (when MeshRenderer component is implemented)
    // auto meshRenderer = go->AddComponent<MeshRenderer>();
    // if (primitiveType == "Cube") {
    //     meshRenderer->SetMesh(PrimitiveMesh::CreateCube());
    // } else if (primitiveType == "Sphere") {
    //     meshRenderer->SetMesh(PrimitiveMesh::CreateSphere());
    // }
    
    return go;
}

} // namespace LGE
