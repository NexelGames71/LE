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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "LGE/rendering/Framebuffer.h"
#include "LGE/math/Vector.h"

// Forward declare GL types
typedef int GLint;

namespace LGE {

class Camera;
class Renderer;
class Texture;
class GameObject;

namespace Luminite {
    class LuminiteSubsystem;
}

class SceneViewport {
public:
    SceneViewport();
    ~SceneViewport();

    void OnUpdate(float deltaTime);
    void OnUIRender();
    
    void BeginRender();  // Begin rendering to framebuffer (binds framebuffer, clears, sets viewport)
    void EndRender();    // End rendering to framebuffer (unbinds framebuffer, restores viewport)

    void SetCamera(Camera* camera) { m_Camera = camera; }
    Camera* GetCamera() const { return m_Camera; }

    bool IsFocused() const { return m_Focused; }
    bool IsHovered() const { return m_Hovered; }
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    
    // Selection and transform
    void SetSelectedObject(GameObject* obj) { m_SelectedObject = obj; }
    GameObject* GetSelectedObject() const { return m_SelectedObject; }
    int GetSelectedTool() const { return m_SelectedTool; }
    void SetGameObjects(const std::vector<std::shared_ptr<GameObject>>& objects) { m_GameObjects = objects; }
    void RenderImGuizmo(); // Render ImGuizmo for selected object
    bool IsGridVisible() const { return m_ShowGrid; }
    bool IsLit() const { return m_IsLit; }
    
    void SetLuminiteSubsystem(Luminite::LuminiteSubsystem* subsystem);

private:
    void LoadIcons(); // Load icon textures
    void HandleMouseInput(); // Handle mouse input for selection and transform
    Math::Vector3 ScreenToWorldRay(float screenX, float screenY); // Convert screen coordinates to world ray
    
    std::unique_ptr<Framebuffer> m_Framebuffer;
    Camera* m_Camera;
    uint32_t m_Width;
    uint32_t m_Height;
    bool m_Focused;
    bool m_Hovered;
    bool m_ViewportSizeChanged;
    GLint m_StoredViewport[4];  // Store viewport to restore after framebuffer rendering
    
    // Icon textures
    std::shared_ptr<Texture> m_TranslateIcon;
    std::shared_ptr<Texture> m_RotateIcon;
    std::shared_ptr<Texture> m_ScaleIcon;
    std::shared_ptr<Texture> m_ShowGridIcon;
    std::shared_ptr<Texture> m_LightActorIcon;
    bool m_IconsLoaded;
    
    // Grid toggle
    bool m_ShowGrid;
    
    // Viewport settings
    bool m_IsLit; // Lighting enabled/disabled
    int m_ProjectionType; // 0=Perspective, 1=Orthographic
    std::shared_ptr<Texture> m_LitIcon;
    
    // Luminite subsystem (for exposure overlay)
    Luminite::LuminiteSubsystem* m_LuminiteSubsystem;
    
    // Selection and transform
    GameObject* m_SelectedObject;
    std::vector<std::shared_ptr<GameObject>> m_GameObjects; // All GameObjects in scene for selection
    int m_SelectedTool; // 0=Translate, 1=Rotate, 2=Scale
    bool m_IsDragging;
    Math::Vector3 m_DragStartPos;
    Math::Vector3 m_DragStartObjectPos;
    Math::Vector3 m_DragStartObjectRot;
    Math::Vector3 m_DragStartObjectScale;
};

} // namespace LGE




