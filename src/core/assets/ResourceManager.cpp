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

#include "LGE/core/assets/ResourceManager.h"
#include "LGE/core/assets/AssetLoader.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/Log.h"
#include <thread>
#include <future>

namespace LGE {

ResourceManager::ResourceManager(AssetLoader* loader, AssetRegistry* registry)
    : m_Loader(loader)
    , m_Registry(registry)
{
}

std::shared_ptr<void> ResourceManager::LoadTexture(const std::string& path) {
    return m_Loader->LoadByPath<void>(path);
}

std::shared_ptr<void> ResourceManager::LoadModel(const std::string& path) {
    return m_Loader->LoadByPath<void>(path);
}

std::shared_ptr<void> ResourceManager::LoadMaterial(const std::string& path) {
    return m_Loader->LoadByPath<void>(path);
}

void ResourceManager::PreloadAssets(const std::vector<std::string>& paths) {
    m_PreloadTotal = static_cast<int>(paths.size());
    m_PreloadCompleted = 0;
    
    // Preload in background thread
    std::thread([this, paths]() {
        for (const auto& path : paths) {
            m_Loader->LoadByPath<void>(path);
            m_PreloadCompleted++;
        }
    }).detach();
}

float ResourceManager::GetLoadingProgress() const {
    if (m_PreloadTotal == 0) {
        return 1.0f;
    }
    return static_cast<float>(m_PreloadCompleted) / static_cast<float>(m_PreloadTotal);
}

bool ResourceManager::IsPreloadingComplete() const {
    return m_PreloadCompleted >= m_PreloadTotal;
}

} // namespace LGE

