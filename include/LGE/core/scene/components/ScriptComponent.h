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

#include "LGE/core/scene/Component.h"
#include "LGE/core/scene/ComponentFactory.h"

namespace LGE {

// Forward declaration
class Collider;

// Base class for user scripts - override virtual methods for behavior
class ScriptComponent : public Component {
public:
    ScriptComponent();
    virtual ~ScriptComponent() = default;
    
    // Component interface
    const char* GetTypeName() const override { return "ScriptComponent"; }
    
    // Override these in derived classes
    virtual void OnAwake() {}
    virtual void OnStart() {}
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnLateUpdate(float deltaTime) {}
    virtual void OnFixedUpdate(float fixedDeltaTime) {}
    
    // Collision events
    virtual void OnCollisionEnter(Collider* other) {}
    virtual void OnCollisionStay(Collider* other) {}
    virtual void OnCollisionExit(Collider* other) {}
    virtual void OnTriggerEnter(Collider* other) {}
    virtual void OnTriggerStay(Collider* other) {}
    virtual void OnTriggerExit(Collider* other) {}
    
    // Mouse events (for UI/raycasting)
    virtual void OnMouseEnter() {}
    virtual void OnMouseOver() {}
    virtual void OnMouseExit() {}
    virtual void OnMouseDown() {}
    virtual void OnMouseUp() {}
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;

protected:
    // Forward lifecycle to virtual methods
    void Awake() override { OnAwake(); }
    void Start() override { OnStart(); }
    void Update(float deltaTime) override { OnUpdate(deltaTime); }
    void LateUpdate(float deltaTime) override { OnLateUpdate(deltaTime); }
    void FixedUpdate(float fixedDeltaTime) override { OnFixedUpdate(fixedDeltaTime); }
};

} // namespace LGE

REGISTER_COMPONENT(ScriptComponent)

