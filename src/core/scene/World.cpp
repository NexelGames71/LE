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

#include "LGE/core/scene/World.h"
#include "LGE/core/scene/GameObject.h"
#include "LGE/core/scene/Component.h"
#include "LGE/core/Log.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <sstream>
#include <functional>
#include <fstream>
#include <chrono>

namespace LGE {

World::World(const std::string& name)
    : m_GUID(GUID::Generate())
    , m_Name(name)
    , m_IsPlaying(false)
    , m_TimeScale(1.0f)
    , m_FixedDeltaTime(0.02f)  // 50 FPS
    , m_FixedTimeAccumulator(0.0f)
{
}

World::~World() {
    Clear();
}

std::shared_ptr<GameObject> World::CreateGameObject(const std::string& name) {
    auto go = GameObject::Create(name);
    AddGameObject(go);
    return go;
}

void World::AddGameObject(std::shared_ptr<GameObject> gameObject) {
    if (!gameObject) return;
    
    gameObject->SetWorld(this);
    
    // If it has no parent, add to root objects
    if (!gameObject->GetParent()) {
        m_RootGameObjects.push_back(gameObject);
    }
    
    // Add to map
    m_GameObjectMap[gameObject->GetGUID()] = gameObject;
    
    // Awake and Start if world is playing
    if (m_IsPlaying) {
        gameObject->Awake();
        gameObject->Start();
    }
}

void World::RemoveGameObject(std::shared_ptr<GameObject> gameObject) {
    if (!gameObject) return;
    
    // Remove from root objects if it's there
    m_RootGameObjects.erase(
        std::remove_if(m_RootGameObjects.begin(), m_RootGameObjects.end(),
            [gameObject](const std::shared_ptr<GameObject>& obj) {
                return obj.get() == gameObject.get();
            }),
        m_RootGameObjects.end()
    );
    
    // Remove from map
    m_GameObjectMap.erase(gameObject->GetGUID());
}

void World::RemoveGameObject(const GUID& guid) {
    auto it = m_GameObjectMap.find(guid);
    if (it != m_GameObjectMap.end()) {
        auto gameObject = it->second.lock();
        if (gameObject) {
            RemoveGameObject(gameObject);
        }
    }
}

void World::DestroyGameObject(std::shared_ptr<GameObject> gameObject) {
    if (!gameObject) return;
    m_PendingDestruction.push_back(gameObject);
}

std::shared_ptr<GameObject> World::FindGameObject(const GUID& guid) const {
    auto it = m_GameObjectMap.find(guid);
    if (it != m_GameObjectMap.end()) {
        return it->second.lock();
    }
    return nullptr;
}

std::shared_ptr<GameObject> World::FindGameObjectByName(const std::string& name) const {
    // Search root objects and their children
    for (const auto& root : m_RootGameObjects) {
        if (root && root->GetName() == name) {
            return root;
        }
        
        auto found = root->FindChildRecursive(name);
        if (found) {
            return found;
        }
    }
    
    return nullptr;
}

std::vector<std::shared_ptr<GameObject>> World::FindGameObjectsByTag(const std::string& tag) const {
    std::vector<std::shared_ptr<GameObject>> result;
    
    std::function<void(std::shared_ptr<GameObject>)> search = [&](std::shared_ptr<GameObject> obj) {
        if (!obj) return;
        
        if (obj->GetTag() == tag) {
            result.push_back(obj);
        }
        
        for (const auto& child : obj->GetChildren()) {
            search(child);
        }
    };
    
    for (const auto& root : m_RootGameObjects) {
        search(root);
    }
    
    return result;
}

std::vector<std::shared_ptr<GameObject>> World::GetAllGameObjects() const {
    std::vector<std::shared_ptr<GameObject>> result;
    
    std::function<void(std::shared_ptr<GameObject>)> collect = [&](std::shared_ptr<GameObject> obj) {
        if (!obj) return;
        
        result.push_back(obj);
        
        for (const auto& child : obj->GetChildren()) {
            collect(child);
        }
    };
    
    for (const auto& root : m_RootGameObjects) {
        collect(root);
    }
    
    return result;
}

void World::Update(float deltaTime) {
    if (!m_IsPlaying) return;
    
    float scaledDeltaTime = deltaTime * m_TimeScale;
    
    // Fixed update accumulation
    m_FixedTimeAccumulator += scaledDeltaTime;
    while (m_FixedTimeAccumulator >= m_FixedDeltaTime) {
        FixedUpdate();
        m_FixedTimeAccumulator -= m_FixedDeltaTime;
    }
    
    // Update all root GameObjects (they will update their children)
    for (auto& gameObject : m_RootGameObjects) {
        if (gameObject && !gameObject->IsDestroyed() && !gameObject->IsStatic()) {
            gameObject->Update(scaledDeltaTime);
        }
    }
    
    // Late update
    for (auto& gameObject : m_RootGameObjects) {
        if (gameObject && !gameObject->IsDestroyed() && !gameObject->IsStatic()) {
            gameObject->LateUpdate(scaledDeltaTime);
        }
    }
    
    // Process destroyed objects
    ProcessPendingDestruction();
    
    // Clean up destroyed objects
    m_RootGameObjects.erase(
        std::remove_if(m_RootGameObjects.begin(), m_RootGameObjects.end(),
            [](const std::shared_ptr<GameObject>& obj) {
                return !obj || obj->IsDestroyed();
            }),
        m_RootGameObjects.end()
    );
    
    UpdateGameObjectMap();
}

void World::LateUpdate(float deltaTime) {
    for (auto& gameObject : m_RootGameObjects) {
        if (gameObject && !gameObject->IsDestroyed()) {
            gameObject->LateUpdate(deltaTime);
        }
    }
}

void World::FixedUpdate() {
    if (!m_IsPlaying) return;
    
    for (auto& gameObject : m_RootGameObjects) {
        if (gameObject && !gameObject->IsDestroyed() && !gameObject->IsStatic()) {
            gameObject->FixedUpdate(m_FixedDeltaTime);
        }
    }
}

void World::Awake() {
    for (auto& gameObject : m_RootGameObjects) {
        if (gameObject) {
            gameObject->Awake();
        }
    }
}

void World::Start() {
    for (auto& gameObject : m_RootGameObjects) {
        if (gameObject && gameObject->IsActiveInHierarchy()) {
            gameObject->Start();
        }
    }
}

void World::Play() {
    if (m_IsPlaying) return;
    
    m_IsPlaying = true;
    m_FixedTimeAccumulator = 0.0f;
    Awake();
    Start();
}

void World::Stop() {
    m_IsPlaying = false;
    m_FixedTimeAccumulator = 0.0f;
}

void World::ProcessPendingDestruction() {
    for (auto& gameObject : m_PendingDestruction) {
        if (gameObject) {
            gameObject->OnDestroy();
            RemoveGameObject(gameObject);
        }
    }
    m_PendingDestruction.clear();
}

void World::Clear() {
    // Destroy all GameObjects
    for (auto& gameObject : m_RootGameObjects) {
        if (gameObject) {
            gameObject->Destroy();
        }
    }
    
    m_RootGameObjects.clear();
    m_GameObjectMap.clear();
}

void World::UpdateGameObjectMap() {
    // Remove expired weak_ptrs
    for (auto it = m_GameObjectMap.begin(); it != m_GameObjectMap.end();) {
        if (it->second.expired()) {
            it = m_GameObjectMap.erase(it);
        } else {
            ++it;
        }
    }
}

std::string World::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"guid\": \"" << m_GUID.ToString() << "\",\n";
    json << "  \"name\": \"" << m_Name << "\",\n";
    json << "  \"version\": 1,\n";
    json << "  \"timeScale\": " << m_TimeScale << ",\n";
    json << "  \"fixedDeltaTime\": " << m_FixedDeltaTime << ",\n";
    
    // Add lastModified timestamp
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    json << "  \"lastModified\": " << static_cast<uint64_t>(timeT) << ",\n";
    
    json << "  \"gameObjects\": [\n";
    
    bool first = true;
    for (const auto& gameObject : m_RootGameObjects) {
        if (gameObject) {
            if (!first) json << ",\n";
            json << "    " << gameObject->Serialize();
            first = false;
        }
    }
    
    json << "\n  ]\n";
    json << "}";
    
    return json.str();
}

bool World::SaveToFile(const std::string& path) const {
    try {
        std::ofstream file(path);
        if (!file.is_open()) {
            return false;
        }
        file << Serialize();
        return true;
    } catch (...) {
        return false;
    }
}

std::shared_ptr<World> World::Deserialize(const std::string& json) {
    try {
        auto j = nlohmann::json::parse(json);
        
        auto world = std::make_shared<World>("DeserializedWorld");
        
        // Parse GUID
        if (j.contains("guid") && j["guid"].is_string()) {
            GUID guid = GUID::FromString(j["guid"]);
            if (guid.IsValid()) {
                world->SetGUID(guid);
            }
        }
        
        // Parse name
        if (j.contains("name") && j["name"].is_string()) {
            world->SetName(j["name"]);
        }
        
        // Parse time scale
        if (j.contains("timeScale") && j["timeScale"].is_number()) {
            world->SetTimeScale(j["timeScale"]);
        }
        
        // Parse fixed delta time
        if (j.contains("fixedDeltaTime") && j["fixedDeltaTime"].is_number()) {
            world->SetFixedDeltaTime(j["fixedDeltaTime"]);
        }
        
        // Parse and restore GameObjects
        if (j.contains("gameObjects") && j["gameObjects"].is_array()) {
            for (const auto& goJson : j["gameObjects"]) {
                if (goJson.is_object()) {
                    std::string goJsonStr = goJson.dump();
                    auto gameObject = GameObject::Deserialize(goJsonStr, world.get());
                    if (gameObject) {
                        world->AddGameObject(gameObject);
                    }
                }
            }
        }
        
        return world;
    } catch (const std::exception& e) {
        Log::Error("World::Deserialize failed: " + std::string(e.what()));
        return nullptr;
    }
}

std::shared_ptr<World> World::LoadFromFile(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            return nullptr;
        }
        
        std::string json((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
        
        return Deserialize(json);
    } catch (...) {
        return nullptr;
    }
}

} // namespace LGE


