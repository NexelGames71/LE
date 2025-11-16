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

#include <memory>
#include <string>

namespace LGE {

// Forward declarations
class GameObject;
class Transform;
class Component;
class MeshRenderer;
class CameraComponent;
class Rigidbody;
class BoxCollider;
class SphereCollider;
class CapsuleCollider;

// Inspector Window - displays and edits GameObject/Component properties
class InspectorWindow {
public:
    InspectorWindow();
    ~InspectorWindow() = default;
    
    // Render the inspector window
    void Render();
    
    // Set target GameObject to inspect
    void SetTarget(std::shared_ptr<GameObject> gameObject);
    
    std::shared_ptr<GameObject> GetTarget() const { return m_TargetGameObject.lock(); }

private:
    // Render GameObject properties
    void RenderGameObjectProperties(std::shared_ptr<GameObject> gameObject);
    
    // Render Transform component
    void RenderTransform(Transform* transform);
    
    // Render a generic component
    void RenderComponent(Component* component);
    
    // Render specific component types
    void RenderMeshRenderer(MeshRenderer* renderer);
    void RenderCamera(CameraComponent* camera);
    void RenderLightComponent(class LightComponent* light);
    void RenderSkyLightComponent(class SkyLightComponent* skyLight);
    void RenderFogComponent(class FogComponent* fog);
    void RenderRigidbody(Rigidbody* rb);
    void RenderBoxCollider(BoxCollider* collider);
    void RenderSphereCollider(SphereCollider* collider);
    void RenderCapsuleCollider(CapsuleCollider* collider);
    
    std::weak_ptr<GameObject> m_TargetGameObject;
};

} // namespace LGE

