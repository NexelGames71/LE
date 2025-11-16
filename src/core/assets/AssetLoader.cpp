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

#include "LGE/core/assets/AssetLoader.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/Log.h"
#include <algorithm>

namespace LGE {

AssetLoader::AssetLoader(AssetRegistry* reg)
    : m_Registry(reg)
{
}

AssetLoader::~AssetLoader() {
    Clear();
}

void AssetLoader::Unload(const GUID& guid) {
    auto it = m_LoadedAssets.find(guid);
    if (it != m_LoadedAssets.end()) {
        m_LoadedAssets.erase(it);
        m_ReferenceCount.erase(guid);
        Log::Info("Unloaded asset: " + guid.ToString());
    }
}

void AssetLoader::UnloadUnused() {
    std::vector<GUID> toUnload;
    
    for (const auto& pair : m_ReferenceCount) {
        if (pair.second <= 0) {
            toUnload.push_back(pair.first);
        }
    }
    
    for (const auto& guid : toUnload) {
        Unload(guid);
    }
    
    if (!toUnload.empty()) {
        Log::Info("Unloaded " + std::to_string(toUnload.size()) + " unused assets");
    }
}

bool AssetLoader::IsLoaded(const GUID& guid) const {
    return m_LoadedAssets.find(guid) != m_LoadedAssets.end();
}

int AssetLoader::GetReferenceCount(const GUID& guid) const {
    auto it = m_ReferenceCount.find(guid);
    if (it != m_ReferenceCount.end()) {
        return it->second;
    }
    return 0;
}

void AssetLoader::OnAssetReleased(const GUID& guid) {
    auto it = m_ReferenceCount.find(guid);
    if (it != m_ReferenceCount.end()) {
        it->second--;
        if (it->second <= 0) {
            // Asset is no longer referenced, but keep it in cache
            // until explicitly unloaded or cache is cleared
            Log::Info("Asset released (reference count: 0): " + guid.ToString());
        }
    }
}

std::shared_ptr<void> AssetLoader::LoadGeneric(const GUID& guid, AssetType type) {
    auto it = m_LoadedAssets.find(guid);
    if (it != m_LoadedAssets.end()) {
        m_ReferenceCount[guid]++;
        return it->second;
    }
    
    AssetMetadata* metadata = m_Registry->GetAsset(guid);
    if (!metadata) return nullptr;
    
    auto loaderIt = m_Loaders.find(type);
    if (loaderIt == m_Loaders.end()) return nullptr;
    
    void* rawAsset = loaderIt->second.loader(guid);
    if (!rawAsset) return nullptr;
    
    auto asset = std::shared_ptr<void>(rawAsset, [this, guid, deleter = loaderIt->second.deleter](void* ptr) {
        OnAssetReleased(guid);
        deleter(ptr);
    });
    
    m_LoadedAssets[guid] = asset;
    m_ReferenceCount[guid] = 1;
    
    return asset;
}

void AssetLoader::Clear() {
    m_LoadedAssets.clear();
    m_ReferenceCount.clear();
    Log::Info("AssetLoader cache cleared");
}

} // namespace LGE

