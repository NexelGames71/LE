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

namespace LGE {

class GameObject;

// Base Component class - all components inherit from this
class Component {
public:
    Component() : m_Owner(nullptr), m_Enabled(true) {}
    virtual ~Component() = default;

    // Called when component is attached to a GameObject
    virtual void OnAttach(GameObject* owner) { m_Owner = owner; }
    
    // Called when component is detached from a GameObject
    virtual void OnDetach() { m_Owner = nullptr; }
    
    // Update called each frame
    virtual void OnUpdate(float deltaTime) {}
    
    // Get the owning GameObject
    GameObject* GetOwner() const { return m_Owner; }
    
    // Enable/disable component
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    bool IsEnabled() const { return m_Enabled; }
    
    // Get component type name (for serialization/reflection)
    virtual const char* GetTypeName() const = 0;

protected:
    GameObject* m_Owner;
    bool m_Enabled;
};

} // namespace LGE



