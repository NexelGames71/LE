/*
------------------------------------------------------------------------------

Luma Engine - Scene Manager Implementation

Copyright (c) 2025 Nexel Games. All Rights Reserved.

This source code is part of the Luma Engine project developed by Nexel Games.

Use of this software is governed by the Luma Engine License Agreement.

Unauthorized copying of this file, via any medium, is strictly prohibited.

Distribution of source or binary forms, with or without modification, is

subject to the terms of the Luma Engine License.

For more information, visit: https://nexelgames.com/luma-engine

------------------------------------------------------------------------------

*/

#include "LGE/core/scene/SceneManager.h"
#include "LGE/core/scene/World.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <fstream>
#include <sstream>
#include <chrono>

namespace LGE {

SceneManager::SceneManager()
    : m_ActiveWorld(nullptr)
{
}

SceneManager::~SceneManager() {
    if (m_ActiveWorld) {
        BroadcastEvent(SceneEvent::WillUnload, m_ActiveWorld);
        BroadcastEvent(SceneEvent::Unloaded, nullptr);
    }
    m_ActiveWorld.reset();
}

bool SceneManager::LoadScene(const std::string& scenePath) {
    if (scenePath.empty()) {
        Log::Error("SceneManager::LoadScene: Scene path is empty");
        return false;
    }
    
    if (!FileSystem::Exists(scenePath)) {
        Log::Error("SceneManager::LoadScene: Scene file does not exist: " + scenePath);
        return false;
    }
    
    // Broadcast will unload event
    if (m_ActiveWorld) {
        BroadcastEvent(SceneEvent::WillUnload, m_ActiveWorld);
    }
    
    // Load the new world
    auto newWorld = World::LoadFromFile(scenePath);
    if (!newWorld) {
        Log::Error("SceneManager::LoadScene: Failed to load scene from: " + scenePath);
        return false;
    }
    
    // Get scene metadata
    m_CurrentSceneMetadata = GetSceneMetadata(scenePath);
    m_CurrentScenePath = scenePath;
    
    // Replace the active world (don't copy - replace the pointer)
    std::shared_ptr<World> oldWorld = m_ActiveWorld;
    m_ActiveWorld = newWorld;
    
    // Broadcast unloaded event for old world
    if (oldWorld) {
        BroadcastEvent(SceneEvent::Unloaded, oldWorld);
    }
    
    // Broadcast will load event
    BroadcastEvent(SceneEvent::WillLoad, m_ActiveWorld);
    
    // Broadcast loaded event
    BroadcastEvent(SceneEvent::Loaded, m_ActiveWorld);
    
    Log::Info("SceneManager: Loaded scene \"" + m_ActiveWorld->GetName() + "\" from " + scenePath);
    
    return true;
}

std::shared_ptr<World> SceneManager::CreateScene(const std::string& name) {
    // Broadcast will unload event
    if (m_ActiveWorld) {
        BroadcastEvent(SceneEvent::WillUnload, m_ActiveWorld);
    }
    
    // Create new world
    auto newWorld = std::make_shared<World>(name);
    
    // Broadcast unloaded event for old world
    if (m_ActiveWorld) {
        BroadcastEvent(SceneEvent::Unloaded, m_ActiveWorld);
    }
    
    // Replace the active world
    m_ActiveWorld = newWorld;
    
    // Update metadata
    m_CurrentSceneMetadata.guid = GUID::Generate();
    m_CurrentSceneMetadata.name = name;
    m_CurrentSceneMetadata.path = "";
    m_CurrentSceneMetadata.version = 1;
    m_CurrentSceneMetadata.timeScale = 1.0f;
    
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    m_CurrentSceneMetadata.lastModified = static_cast<uint64_t>(timeT);
    
    m_CurrentScenePath = "";
    
    // Broadcast events
    BroadcastEvent(SceneEvent::WillLoad, m_ActiveWorld);
    BroadcastEvent(SceneEvent::Loaded, m_ActiveWorld);
    
    Log::Info("SceneManager: Created new scene \"" + name + "\"");
    
    return m_ActiveWorld;
}

bool SceneManager::ReloadCurrentScene() {
    if (m_CurrentScenePath.empty()) {
        Log::Error("SceneManager::ReloadCurrentScene: No scene is currently loaded");
        return false;
    }
    
    return LoadScene(m_CurrentScenePath);
}

bool SceneManager::SaveCurrentScene() {
    if (!m_ActiveWorld) {
        Log::Error("SceneManager::SaveCurrentScene: No active world to save");
        return false;
    }
    
    if (m_CurrentScenePath.empty()) {
        Log::Error("SceneManager::SaveCurrentScene: No scene path set");
        return false;
    }
    
    return SaveScene(m_ActiveWorld, m_CurrentScenePath);
}

bool SceneManager::SaveScene(std::shared_ptr<World> world, const std::string& path) {
    if (!world) {
        Log::Error("SceneManager::SaveScene: World is null");
        return false;
    }
    
    if (path.empty()) {
        Log::Error("SceneManager::SaveScene: Path is empty");
        return false;
    }
    
    // Update metadata
    UpdateSceneMetadata(world, path);
    
    // Save the world
    if (!world->SaveToFile(path)) {
        Log::Error("SceneManager::SaveScene: Failed to save world to: " + path);
        return false;
    }
    
    // Update current scene path if this is the active world
    if (world == m_ActiveWorld) {
        m_CurrentScenePath = path;
    }
    
    Log::Info("SceneManager: Saved scene \"" + world->GetName() + "\" to " + path);
    
    return true;
}

SceneMetadata SceneManager::GetSceneMetadata(const std::string& scenePath) {
    SceneMetadata metadata;
    
    if (!FileSystem::Exists(scenePath)) {
        return metadata;
    }
    
    // Read file and parse metadata
    std::string json = FileSystem::ReadFile(scenePath);
    if (json.empty()) {
        return metadata;
    }
    
    // Parse JSON to extract metadata
    // Look for "guid", "name", "version", "lastModified", "timeScale"
    size_t guidPos = json.find("\"guid\"");
    if (guidPos != std::string::npos) {
        size_t colonPos = json.find(':', guidPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                std::string guidStr = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                metadata.guid = GUID::FromString(guidStr);
            }
        }
    }
    
    size_t namePos = json.find("\"name\"");
    if (namePos != std::string::npos) {
        size_t colonPos = json.find(':', namePos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                metadata.name = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Extract version, lastModified, timeScale similarly
    // For now, set defaults
    metadata.version = 1;
    metadata.timeScale = 1.0f;
    
    // Get file modification time
    // This is a simplified version - in a real implementation you'd use proper file system APIs
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    metadata.lastModified = static_cast<uint64_t>(timeT);
    
    metadata.path = scenePath;
    
    return metadata;
}

void SceneManager::RegisterSceneEventCallback(SceneEventCallback callback) {
    if (callback) {
        m_EventCallbacks.push_back(callback);
    }
}

void SceneManager::UnregisterSceneEventCallback(SceneEventCallback callback) {
    // Remove callback (simplified - in practice you'd want a more robust way to identify callbacks)
    m_EventCallbacks.erase(
        std::remove_if(m_EventCallbacks.begin(), m_EventCallbacks.end(),
            [&callback](const SceneEventCallback& cb) {
                // Compare function pointers (this is a simplified approach)
                return &cb == &callback;
            }),
        m_EventCallbacks.end()
    );
}

void SceneManager::BroadcastEvent(SceneEvent event, std::shared_ptr<World> world) {
    for (auto& callback : m_EventCallbacks) {
        if (callback) {
            callback(event, world);
        }
    }
}

void SceneManager::UpdateSceneMetadata(std::shared_ptr<World> world, const std::string& path) {
    if (!world) return;
    
    // Generate GUID if not set
    if (!m_CurrentSceneMetadata.guid.IsValid()) {
        m_CurrentSceneMetadata.guid = GUID::Generate();
    }
    
    m_CurrentSceneMetadata.name = world->GetName();
    m_CurrentSceneMetadata.path = path;
    m_CurrentSceneMetadata.timeScale = world->GetTimeScale();
    
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    m_CurrentSceneMetadata.lastModified = static_cast<uint64_t>(timeT);
}

} // namespace LGE

