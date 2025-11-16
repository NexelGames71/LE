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

// Template implementations for World component queries
// Note: This file is included after the World class definition in World.h

template<typename T>
T* LGE::World::FindObjectOfType() {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    for (auto& go : m_RootGameObjects) {
        if (!go || go->IsDestroyed()) continue;
        
        auto component = go->GetComponent<T>();
        if (component) {
            return component;
        }
        
        // Search children recursively
        std::function<T*(std::shared_ptr<GameObject>)> search = [](std::shared_ptr<GameObject> obj) -> T* {
            if (!obj || obj->IsDestroyed()) return nullptr;
            
            auto comp = obj->GetComponent<T>();
            if (comp) return comp;
            
            for (const auto& child : obj->GetChildren()) {
                auto found = search(child);
                if (found) return found;
            }
            
            return nullptr;
        };
        
        auto found = search(go);
        if (found) return found;
    }
    
    return nullptr;
}

template<typename T>
std::vector<T*> LGE::World::FindObjectsOfType() {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    
    std::vector<T*> result;
    
    std::function<void(std::shared_ptr<GameObject>)> collect = [&](std::shared_ptr<GameObject> obj) {
        if (!obj || obj->IsDestroyed()) return;
        
        auto components = obj->GetComponents<T>();
        result.insert(result.end(), components.begin(), components.end());
        
        for (const auto& child : obj->GetChildren()) {
            collect(child);
        }
    };
    
    for (auto& go : m_RootGameObjects) {
        collect(go);
    }
    
    return result;
}

