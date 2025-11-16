/*
------------------------------------------------------------------------------

Luma Engine - Scene Manager

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
#include <functional>
#include <vector>
#include "LGE/core/GUID.h"

namespace LGE {

class World;

// Scene lifecycle events
enum class SceneEvent {
    WillLoad,
    Loaded,
    WillUnload,
    Unloaded,
    PlayModeEnter,
    PlayModeExit
};

// Scene metadata
struct SceneMetadata {
    GUID guid;
    std::string name;
    std::string path;
    uint32_t version;
    uint64_t lastModified;
    float timeScale;
    
    SceneMetadata()
        : guid(GUID::Invalid())
        , version(1)
        , lastModified(0)
        , timeScale(1.0f)
    {}
};

// Scene Manager - handles scene loading, unloading, and lifecycle
class SceneManager {
public:
    SceneManager();
    ~SceneManager();
    
    // Get active world
    std::shared_ptr<World> GetActiveWorld() const { return m_ActiveWorld; }
    
    // Load scene from file (replaces current world)
    bool LoadScene(const std::string& scenePath);
    
    // Create new empty scene
    std::shared_ptr<World> CreateScene(const std::string& name);
    
    // Reload current scene
    bool ReloadCurrentScene();
    
    // Save current scene
    bool SaveCurrentScene();
    
    // Save scene to specific path
    bool SaveScene(std::shared_ptr<World> world, const std::string& path);
    
    // Get current scene metadata
    const SceneMetadata& GetCurrentSceneMetadata() const { return m_CurrentSceneMetadata; }
    
    // Get scene metadata from file
    static SceneMetadata GetSceneMetadata(const std::string& scenePath);
    
    // Scene event callbacks
    using SceneEventCallback = std::function<void(SceneEvent, std::shared_ptr<World>)>;
    void RegisterSceneEventCallback(SceneEventCallback callback);
    void UnregisterSceneEventCallback(SceneEventCallback callback);
    
    // Check if scene is loaded
    bool IsSceneLoaded() const { return m_ActiveWorld != nullptr; }
    
    // Get current scene path
    const std::string& GetCurrentScenePath() const { return m_CurrentScenePath; }

private:
    void BroadcastEvent(SceneEvent event, std::shared_ptr<World> world);
    void UpdateSceneMetadata(std::shared_ptr<World> world, const std::string& path);
    
    std::shared_ptr<World> m_ActiveWorld;
    std::string m_CurrentScenePath;
    SceneMetadata m_CurrentSceneMetadata;
    
    std::vector<SceneEventCallback> m_EventCallbacks;
};

} // namespace LGE

