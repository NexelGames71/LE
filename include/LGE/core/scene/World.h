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
#include <string>
#include <unordered_map>
#include "LGE/core/GUID.h"

namespace LGE {

class GameObject;

// World/Scene class for managing GameObjects
class World {
public:
    World(const std::string& name = "World");
    ~World();
    
    // World GUID
    const GUID& GetGUID() const { return m_GUID; }
    void SetGUID(const GUID& guid) { m_GUID = guid; }
    
    // World name
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }
    
    // GameObject management
    void AddGameObject(std::shared_ptr<GameObject> gameObject);
    void RemoveGameObject(std::shared_ptr<GameObject> gameObject);
    void RemoveGameObject(const GUID& guid);
    void DestroyGameObject(std::shared_ptr<GameObject> gameObject);
    
    // Find GameObjects
    std::shared_ptr<GameObject> FindGameObject(const GUID& guid) const;
    std::shared_ptr<GameObject> FindGameObjectByName(const std::string& name) const;
    std::vector<std::shared_ptr<GameObject>> FindGameObjectsByTag(const std::string& tag) const;
    
    // Get all root GameObjects (no parent)
    const std::vector<std::shared_ptr<GameObject>>& GetRootGameObjects() const { return m_RootGameObjects; }
    
    // Get all GameObjects
    std::vector<std::shared_ptr<GameObject>> GetAllGameObjects() const;
    
    // GameObject creation
    std::shared_ptr<GameObject> CreateGameObject(const std::string& name = "GameObject");
    
    // Update loop
    void Update(float deltaTime);
    void LateUpdate(float deltaTime);
    void FixedUpdate();
    
    // Lifecycle
    void Awake();
    void Start();
    
    // Play mode
    bool IsPlaying() const { return m_IsPlaying; }
    void Play();
    void Stop();
    
    // Time scale
    float GetTimeScale() const { return m_TimeScale; }
    void SetTimeScale(float scale) { m_TimeScale = scale; }
    
    float GetFixedDeltaTime() const { return m_FixedDeltaTime; }
    void SetFixedDeltaTime(float dt) { m_FixedDeltaTime = dt; }
    
    // Find by component type
    template<typename T>
    T* FindObjectOfType();
    
    template<typename T>
    std::vector<T*> FindObjectsOfType();
    
    // Clear all GameObjects
    void Clear();
    
    // Serialization
    std::string Serialize() const;
    bool SaveToFile(const std::string& path) const;
    static std::shared_ptr<World> Deserialize(const std::string& json);
    static std::shared_ptr<World> LoadFromFile(const std::string& path);

private:
    GUID m_GUID;
    std::string m_Name;
    std::vector<std::shared_ptr<GameObject>> m_RootGameObjects;
    std::unordered_map<GUID, std::weak_ptr<GameObject>> m_GameObjectMap;
    
    // Play mode state
    bool m_IsPlaying;
    float m_TimeScale;
    float m_FixedDeltaTime;
    float m_FixedTimeAccumulator;
    
    // Objects pending destruction
    std::vector<std::shared_ptr<GameObject>> m_PendingDestruction;
    
    void UpdateGameObjectMap();
    void ProcessPendingDestruction();
};

// Template implementations
#include "LGE/core/scene/World.inl"

} // namespace LGE

