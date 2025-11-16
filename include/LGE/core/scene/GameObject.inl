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

// Template implementations for GameObject component management
// Note: This file is included inside the LGE namespace in GameObject.h

template<typename T, typename... Args>
T* GameObject::AddComponent(Args&&... args) {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    // Check if this is a unique component type (optional - can be implemented per component)
    // For now, we allow multiple components of the same type
    
    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = component.get();
    component->OnAttach(this);
    
    // Store in component map
    std::type_index typeIndex(typeid(T));
    m_Components[typeIndex] = std::move(component);
    
    // Note: Transform component caching is handled in GameObject.cpp
    // to avoid circular dependency issues
    
    // Call Awake if game object has already started
    if (m_HasStarted && IsActiveInHierarchy()) {
        ptr->Awake();
        ptr->Start();
        ptr->SetHasStarted(true);
    }
    
    return ptr;
}

template<typename T>
T* GameObject::GetComponent() {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    std::type_index typeIndex(typeid(T));
    auto it = m_Components.find(typeIndex);
    
    if (it != m_Components.end()) {
        return static_cast<T*>(it->second.get());
    }
    
    return nullptr;
}

template<typename T>
const T* GameObject::GetComponent() const {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    std::type_index typeIndex(typeid(T));
    auto it = m_Components.find(typeIndex);
    
    if (it != m_Components.end()) {
        return static_cast<const T*>(it->second.get());
    }
    
    return nullptr;
}

template<typename T>
std::vector<T*> GameObject::GetComponents() {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    std::vector<T*> result;
    std::type_index typeIndex(typeid(T));
    
    // Note: With current implementation, we only store one component per type
    // This can be extended to support multiple components of the same type
    auto it = m_Components.find(typeIndex);
    if (it != m_Components.end()) {
        result.push_back(static_cast<T*>(it->second.get()));
    }
    
    return result;
}

template<typename T>
std::vector<const T*> GameObject::GetComponents() const {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    std::vector<const T*> result;
    std::type_index typeIndex(typeid(T));
    
    auto it = m_Components.find(typeIndex);
    if (it != m_Components.end()) {
        result.push_back(static_cast<const T*>(it->second.get()));
    }
    
    return result;
}

template<typename T>
T* GameObject::GetComponentInChildren() {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    // Check self first
    T* comp = GetComponent<T>();
    if (comp) return comp;
    
    // Check children recursively
    for (const auto& child : m_Children) {
        if (child) {
            comp = child->GetComponentInChildren<T>();
            if (comp) return comp;
        }
    }
    
    return nullptr;
}

template<typename T>
const T* GameObject::GetComponentInChildren() const {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    // Check self first
    const T* comp = GetComponent<T>();
    if (comp) return comp;
    
    // Check children recursively
    for (const auto& child : m_Children) {
        if (child) {
            comp = child->GetComponentInChildren<T>();
            if (comp) return comp;
        }
    }
    
    return nullptr;
}

template<typename T>
T* GameObject::GetComponentInParent() {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    // Check self first
    T* comp = GetComponent<T>();
    if (comp) return comp;
    
    // Check parent recursively
    auto parent = m_Parent.lock();
    if (parent) {
        return parent->GetComponentInParent<T>();
    }
    
    return nullptr;
}

template<typename T>
const T* GameObject::GetComponentInParent() const {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    // Check self first
    const T* comp = GetComponent<T>();
    if (comp) return comp;
    
    // Check parent recursively
    auto parent = m_Parent.lock();
    if (parent) {
        return parent->GetComponentInParent<T>();
    }
    
    return nullptr;
}

template<typename T>
bool GameObject::HasComponent() const {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    std::type_index typeIndex(typeid(T));
    return m_Components.find(typeIndex) != m_Components.end();
}

template<typename T>
void GameObject::RemoveComponent() {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    std::type_index typeIndex(typeid(T));
    auto it = m_Components.find(typeIndex);
    
    if (it != m_Components.end()) {
        it->second->OnDestroy();
        it->second->OnDetach();
        
        // Note: Transform cache clearing is handled in GameObject.cpp
        // to avoid circular dependency issues
        
        m_Components.erase(it);
    }
}

