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
#include <vector>
#include <functional>

namespace LGE {

// Forward declarations
class World;
class GameObject;

// Hierarchy Window - displays GameObject hierarchy tree
class HierarchyWindow {
public:
    HierarchyWindow(World* world);
    ~HierarchyWindow() = default;
    
    // Render the hierarchy window
    void Render();
    
    // Get currently selected GameObject
    std::shared_ptr<GameObject> GetSelectedGameObject() const;
    
    // Set selected GameObject
    void SetSelectedGameObject(std::shared_ptr<GameObject> gameObject);

private:
    // Render a single GameObject node in the tree
    void RenderGameObjectNode(std::shared_ptr<GameObject> gameObject);
    
    // Show context menu for GameObject
    void ShowContextMenu(std::shared_ptr<GameObject> gameObject);
    
    World* m_World;
    std::weak_ptr<GameObject> m_SelectedGameObject;
};

} // namespace LGE

