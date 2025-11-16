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

#include "LGE/core/GUID.h"
#include "LGE/core/assets/AssetMetadata.h"
#include <memory>
#include <unordered_map>
#include <functional>

namespace LGE {

// Forward declarations
class AssetRegistry;

class AssetLoader {
private:
    AssetRegistry* m_Registry;
    
    // Type-specific loaders and deleters
    struct LoaderInfo {
        std::function<void*(const GUID&)> loader;
        std::function<void(void*)> deleter;
    };
    std::unordered_map<AssetType, LoaderInfo> m_Loaders;
    
    // Loaded asset cache
    std::unordered_map<GUID, std::shared_ptr<void>> m_LoadedAssets;
    std::unordered_map<GUID, int> m_ReferenceCount;

public:
    AssetLoader(AssetRegistry* reg);
    ~AssetLoader();
    
    // Register type-specific loader
    template<typename T>
    void RegisterLoader(AssetType type, std::function<T*(const GUID&)> loader) {
        LoaderInfo info;
        info.loader = [loader](const GUID& guid) -> void* {
            return static_cast<void*>(loader(guid));
        };
        info.deleter = [](void* ptr) {
            delete static_cast<T*>(ptr);
        };
        m_Loaders[type] = info;
    }
    
    // Load asset by GUID
    template<typename T>
    std::shared_ptr<T> Load(const GUID& guid) {
        auto it = m_LoadedAssets.find(guid);
        if (it != m_LoadedAssets.end()) {
            m_ReferenceCount[guid]++;
            return std::static_pointer_cast<T>(it->second);
        }
        
        AssetMetadata* metadata = m_Registry->GetAsset(guid);
        if (!metadata) return nullptr;
        
        auto loaderIt = m_Loaders.find(metadata->type);
        if (loaderIt == m_Loaders.end()) return nullptr;
        
        T* rawAsset = static_cast<T*>(loaderIt->second.loader(guid));
        if (!rawAsset) return nullptr;
        
        auto asset = std::shared_ptr<T>(rawAsset, [this, guid](T* ptr) {
            OnAssetReleased(guid);
            delete ptr;
        });
        
        m_LoadedAssets[guid] = std::static_pointer_cast<void>(asset);
        m_ReferenceCount[guid] = 1;
        
        return asset;
    }
    
    // Load by path
    template<typename T>
    std::shared_ptr<T> LoadByPath(const std::string& virtualPath) {
        AssetMetadata* metadata = m_Registry->GetAssetByPath(virtualPath);
        if (!metadata) return nullptr;
        return Load<T>(metadata->guid);
    }
    
    // Unload asset
    void Unload(const GUID& guid);
    
    // Unload all unused assets
    void UnloadUnused();
    
    // Check if asset is loaded
    bool IsLoaded(const GUID& guid) const;
    
    // Get reference count
    int GetReferenceCount(const GUID& guid) const;
    
    // Get loaded asset count
    size_t GetLoadedAssetCount() const { return m_LoadedAssets.size(); }
    
    // Clear all loaded assets
    void Clear();
    
    // Get registry
    AssetRegistry* GetRegistry() const { return m_Registry; }
    
    // Non-template load method for async loading
    std::shared_ptr<void> LoadGeneric(const GUID& guid, AssetType type);

private:
    void OnAssetReleased(const GUID& guid);
};

} // namespace LGE

