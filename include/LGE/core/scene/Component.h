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
#include <memory>
#include <vector>
#include "LGE/core/GUID.h"

namespace LGE {

class GameObject;
class Transform;

// Base Component class - all components inherit from this
class Component {
public:
    Component();
    virtual ~Component() = default;

    // GUID
    const GUID& GetGUID() const { return m_GUID; }
    
    // Called when component is attached to a GameObject
    virtual void OnAttach(GameObject* owner) { m_Owner = owner; }
    
    // Called when component is detached from a GameObject
    virtual void OnDetach() { m_Owner = nullptr; }
    
    // Get the owning GameObject
    GameObject* GetOwner() const { return m_Owner; }
    GameObject* GetGameObject() const { return m_Owner; }
    void SetOwner(GameObject* owner) { m_Owner = owner; }
    
    // Enable/disable component
    void SetEnabled(bool enabled);
    bool IsEnabled() const { return m_Enabled; }
    
    // Check if component has started
    bool HasStarted() const { return m_HasStarted; }
    void SetHasStarted(bool started) { m_HasStarted = started; }
    
    // Component type info
    virtual const char* GetTypeName() const = 0;
    virtual bool IsUnique() const { return false; }  // Can only have one per GameObject
    
    // Lifecycle methods (called in order)
    virtual void Awake() {}           // Called immediately after component is created
    virtual void Start() {}             // Called before first Update, after all Awake calls
    virtual void Update(float deltaTime) {}  // Called every frame
    virtual void LateUpdate(float deltaTime) {}  // Called after all Update calls
    virtual void FixedUpdate(float fixedDeltaTime) {}  // Called at fixed intervals for physics
    virtual void OnEnable() {}         // Called when component is enabled
    virtual void OnDisable() {}        // Called when component is disabled
    virtual void OnDestroy() {}         // Called when component is being destroyed
    
    // Helper methods to get components from owner
    template<typename T>
    T* GetComponent() const;
    
    template<typename T>
    std::vector<T*> GetComponents() const;
    
    Transform* GetTransform() const;
    
    // Serialization
    virtual std::string Serialize() const;
    virtual void Deserialize(const std::string& json);

protected:
    GUID m_GUID;
    GameObject* m_Owner;
    bool m_Enabled;
    bool m_HasStarted;
};

// Template implementations
#include "LGE/core/scene/Component.inl"

} // namespace LGE



