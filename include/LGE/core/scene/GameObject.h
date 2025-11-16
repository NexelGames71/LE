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

#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <algorithm>

#include "LGE/core/GUID.h"
#include "LGE/math/Vector.h"
#include "LGE/math/Matrix.h"
#include "LGE/core/scene/Component.h"  // Include Component fully for implementation

namespace LGE {

class World;
class Transform;

// Forward declaration for shared_ptr
class GameObject;

class GameObject : public std::enable_shared_from_this<GameObject> {
public:
    GameObject(const std::string& name = "GameObject");
    ~GameObject();

    // GUID
    const GUID& GetGUID() const { return m_GUID; }
    
    // Name
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }
    
    // Tag
    const std::string& GetTag() const { return m_Tag; }
    void SetTag(const std::string& tag) { m_Tag = tag; }
    
    // Layer
    uint32_t GetLayer() const { return m_Layer; }
    void SetLayer(uint32_t layer) { m_Layer = layer; }
    
    // Active state
    bool IsActive() const { return m_IsActive; }
    void SetActive(bool active);
    bool IsActiveInHierarchy() const;
    
    // Static flag
    bool IsStatic() const { return m_IsStatic; }
    void SetStatic(bool staticFlag) { m_IsStatic = staticFlag; }
    
    // World reference
    World* GetWorld() const { return m_World; }
    void SetWorld(World* world) { m_World = world; }
    
    // Transform (legacy - for backward compatibility)
    void SetPosition(const Math::Vector3& position);
    void SetRotation(const Math::Vector3& rotation);
    void SetScale(const Math::Vector3& scale);
    Math::Vector3 GetPosition() const;
    Math::Vector3 GetRotation() const;
    Math::Vector3 GetScale() const;
    const Math::Matrix4& GetTransformMatrix();
    
    // Transform Component (preferred)
    Transform* GetTransform() const;
    
    // Selection
    void SetSelected(bool selected) { m_Selected = selected; }
    bool IsSelected() const { return m_Selected; }
    
    // ID for picking (legacy)
    uint32_t GetID() const { return m_ID; }
    
    // Hierarchy management
    std::shared_ptr<GameObject> GetParent() const { return m_Parent.lock(); }
    void SetParent(std::shared_ptr<GameObject> parent);
    void RemoveParent();
    
    const std::vector<std::shared_ptr<GameObject>>& GetChildren() const { return m_Children; }
    std::shared_ptr<GameObject> GetChild(size_t index) const;
    std::shared_ptr<GameObject> FindChild(const std::string& name) const;
    std::shared_ptr<GameObject> FindChildRecursive(const std::string& name) const;
    size_t GetChildCount() const { return m_Children.size(); }
    
    // Component system
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args);
    
    template<typename T>
    T* GetComponent();
    
    template<typename T>
    const T* GetComponent() const;
    
    template<typename T>
    std::vector<T*> GetComponents();
    
    template<typename T>
    std::vector<const T*> GetComponents() const;
    
    template<typename T>
    T* GetComponentInChildren();
    
    template<typename T>
    const T* GetComponentInChildren() const;
    
    template<typename T>
    T* GetComponentInParent();
    
    template<typename T>
    const T* GetComponentInParent() const;
    
    template<typename T>
    bool HasComponent() const;
    
    template<typename T>
    void RemoveComponent();
    
    bool RemoveComponent(Component* component);
    
    // Add component from unique_ptr (for deserialization)
    Component* AddComponent(std::unique_ptr<Component> component);
    
    const std::unordered_map<std::type_index, std::unique_ptr<Component>>& GetAllComponents() const { return m_Components; }
    
    // Update all components
    void UpdateComponents(float deltaTime);
    
    // Lifecycle methods
    void Awake();
    void Start();
    void Update(float deltaTime);
    void LateUpdate(float deltaTime);
    void FixedUpdate(float fixedDeltaTime);
    void OnDestroy();
    
    // Destruction
    void Destroy();
    bool IsDestroyed() const { return m_IsDestroyed; }
    
    // Serialization
    std::string Serialize() const;
    static std::shared_ptr<GameObject> Deserialize(const std::string& json, World* world);
    
    // Static factory methods
    static std::shared_ptr<GameObject> Create(const std::string& name = "GameObject");
    static std::shared_ptr<GameObject> CreatePrimitive(const std::string& primitiveType);

private:
    void UpdateTransformMatrix();
    void NotifyComponentsActiveChanged(bool active);
    
    GUID m_GUID;
    std::string m_Name;
    std::string m_Tag;
    uint32_t m_Layer;
    bool m_IsActive;
    bool m_IsStatic;
    bool m_Selected;
    bool m_IsDestroyed;
    bool m_HasStarted;
    
    uint32_t m_ID;  // Legacy ID for picking
    
    // Hierarchy
    std::weak_ptr<GameObject> m_Parent;
    std::vector<std::shared_ptr<GameObject>> m_Children;
    
    // Transform (legacy - kept for backward compatibility)
    Math::Vector3 m_Position;
    Math::Vector3 m_Rotation;
    Math::Vector3 m_Scale;
    Math::Matrix4 m_TransformMatrix;
    bool m_TransformDirty;
    
    // Components
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_Components;
    Transform* m_TransformComponent;  // Cached pointer to Transform component
    
    // World reference
    World* m_World;
    
    static uint32_t s_NextID;
};

// Template implementations
#include "LGE/core/scene/GameObject.inl"

} // namespace LGE
