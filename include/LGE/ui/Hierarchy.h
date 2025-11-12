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

#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace LGE {

class GameObject;
class Texture;

class Hierarchy {
public:
    Hierarchy();
    ~Hierarchy();

    void OnUIRender();
    
    void SetGameObjects(const std::vector<std::shared_ptr<GameObject>>& objects) { m_GameObjects = objects; }
    void SetSelectedObject(GameObject* obj) { m_SelectedObject = obj; }
    GameObject* GetSelectedObject() const { return m_SelectedObject; }
    
    // Callback for creating new GameObjects (called with GameObject type name)
    void SetOnCreateGameObject(std::function<void(const std::string&)> callback) { m_OnCreateGameObject = callback; }

private:
    void LoadIcons();
    
    std::vector<std::shared_ptr<GameObject>> m_GameObjects;
    GameObject* m_SelectedObject;
    
    // Icons
    std::shared_ptr<Texture> m_PlusIcon;
    std::shared_ptr<Texture> m_SearchIcon;
    std::shared_ptr<Texture> m_LightActorIcon;
    bool m_IconsLoaded;
    
    // Callback for creating GameObjects
    std::function<void(const std::string&)> m_OnCreateGameObject;
};

} // namespace LGE


