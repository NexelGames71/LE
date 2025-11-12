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

#include "LGE/math/Vector.h"
#include "LGE/math/Matrix.h"

namespace LGE {

class Component;

class GameObject {
public:
    GameObject(const std::string& name = "GameObject");
    ~GameObject();

    // Transform
    void SetPosition(const Math::Vector3& position) { m_Position = position; m_TransformDirty = true; }
    void SetRotation(const Math::Vector3& rotation) { m_Rotation = rotation; m_TransformDirty = true; }
    void SetScale(const Math::Vector3& scale) { m_Scale = scale; m_TransformDirty = true; }
    
    Math::Vector3 GetPosition() const { return m_Position; }
    Math::Vector3 GetRotation() const { return m_Rotation; }
    Math::Vector3 GetScale() const { return m_Scale; }
    
    const Math::Matrix4& GetTransformMatrix();
    
    // Selection
    void SetSelected(bool selected) { m_Selected = selected; }
    bool IsSelected() const { return m_Selected; }
    
    // Name
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }
    
    // ID for picking
    uint32_t GetID() const { return m_ID; }
    
    // Component system
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();
        component->OnAttach(this);
        m_Components[std::type_index(typeid(T))] = std::move(component);
        return ptr;
    }
    
    template<typename T>
    T* GetComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        auto it = m_Components.find(std::type_index(typeid(T)));
        if (it != m_Components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }
    
    template<typename T>
    const T* GetComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        auto it = m_Components.find(std::type_index(typeid(T)));
        if (it != m_Components.end()) {
            return static_cast<const T*>(it->second.get());
        }
        return nullptr;
    }
    
    template<typename T>
    bool HasComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        return m_Components.find(std::type_index(typeid(T))) != m_Components.end();
    }
    
    template<typename T>
    void RemoveComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        auto it = m_Components.find(std::type_index(typeid(T)));
        if (it != m_Components.end()) {
            it->second->OnDetach();
            m_Components.erase(it);
        }
    }
    
    // Update all components
    void UpdateComponents(float deltaTime);

private:
    void UpdateTransformMatrix();
    
    std::string m_Name;
    uint32_t m_ID;
    
    Math::Vector3 m_Position;
    Math::Vector3 m_Rotation; // Euler angles in degrees
    Math::Vector3 m_Scale;
    
    Math::Matrix4 m_TransformMatrix;
    bool m_TransformDirty;
    bool m_Selected;
    
    // Component storage
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_Components;
    
    static uint32_t s_NextID;
};

} // namespace LGE

