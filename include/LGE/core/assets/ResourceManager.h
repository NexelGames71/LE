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

#include "LGE/core/assets/AssetLoader.h"
#include <string>
#include <vector>
#include <memory>
#include <future>
#include <atomic>

namespace LGE {

// Forward declarations for engine types
class Texture;
class Model;
class Material;

// Forward declarations
class AssetLoader;
class AssetRegistry;

class ResourceManager {
private:
    AssetLoader* m_Loader;
    AssetRegistry* m_Registry;
    
    // Async preloading
    std::atomic<int> m_PreloadTotal{0};
    std::atomic<int> m_PreloadCompleted{0};

public:
    ResourceManager(AssetLoader* loader, AssetRegistry* registry);
    
    // Convenient load functions with path strings
    // Note: These return shared_ptr, but the actual types need to be defined
    // For now, we'll use void* and let the user cast, or define these when types are available
    
    // Load texture by path
    std::shared_ptr<void> LoadTexture(const std::string& path);
    
    // Load model by path
    std::shared_ptr<void> LoadModel(const std::string& path);
    
    // Load material by path
    std::shared_ptr<void> LoadMaterial(const std::string& path);
    
    // Generic load by path
    template<typename T>
    std::shared_ptr<T> Load(const std::string& path) {
        return m_Loader->LoadByPath<T>(path);
    }
    
    // Preload assets (async)
    void PreloadAssets(const std::vector<std::string>& paths);
    
    // Get loading progress (0.0 to 1.0)
    float GetLoadingProgress() const;
    
    // Check if preloading is complete
    bool IsPreloadingComplete() const;
    
    // Get loader
    AssetLoader* GetLoader() const { return m_Loader; }
    
    // Get registry
    AssetRegistry* GetRegistry() const { return m_Registry; }
};

} // namespace LGE

