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

#include "LGE/core/assets/AssetMetadata.h"
#include "LGE/core/GUID.h"
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <functional>

namespace LGE {

class AssetRegistry {
private:
    std::unordered_map<GUID, AssetMetadata> m_Assets;
    std::unordered_map<std::string, GUID> m_PathIndex;
    std::multimap<AssetType, GUID> m_TypeIndex;
    std::multimap<std::string, GUID> m_TagIndex;
    
    std::filesystem::path m_CacheFilePath;
    bool m_IsDirty;

public:
    AssetRegistry();
    ~AssetRegistry();
    
    // Asset management
    bool RegisterAsset(const AssetMetadata& metadata);
    bool UnregisterAsset(const GUID& guid);
    bool UpdateAsset(const GUID& guid, const AssetMetadata& metadata);
    
    // Queries
    AssetMetadata* GetAsset(const GUID& guid);
    const AssetMetadata* GetAsset(const GUID& guid) const;
    AssetMetadata* GetAssetByPath(const std::string& virtualPath);
    const AssetMetadata* GetAssetByPath(const std::string& virtualPath) const;
    std::vector<AssetMetadata*> GetAssetsByType(AssetType type);
    std::vector<const AssetMetadata*> GetAssetsByType(AssetType type) const;
    std::vector<AssetMetadata*> GetAssetsByTag(const std::string& tag);
    std::vector<const AssetMetadata*> GetAssetsByTag(const std::string& tag) const;
    std::vector<AssetMetadata*> SearchAssets(const std::string& query);
    std::vector<const AssetMetadata*> SearchAssets(const std::string& query) const;
    
    // Dependency tracking
    std::vector<GUID> GetDependencies(const GUID& guid);
    std::vector<GUID> GetDependents(const GUID& guid); // What uses this asset
    
    // Persistence
    bool SaveToCache();
    bool LoadFromCache();
    void SetCacheFilePath(const std::filesystem::path& path) { m_CacheFilePath = path; }
    std::filesystem::path GetCacheFilePath() const { return m_CacheFilePath; }
    
    // Statistics
    size_t GetAssetCount() const { return m_Assets.size(); }
    size_t GetAssetCountByType(AssetType type) const;
    
    // Clear all assets
    void Clear();
    
    // Check if registry is dirty (needs saving)
    bool IsDirty() const { return m_IsDirty; }
    void SetDirty(bool dirty) { m_IsDirty = dirty; }

private:
    // Helper methods for indexing
    void AddToIndices(const GUID& guid, const AssetMetadata& metadata);
    void RemoveFromIndices(const GUID& guid, const AssetMetadata& metadata);
    void UpdateIndices(const GUID& guid, const AssetMetadata& oldMetadata, const AssetMetadata& newMetadata);
};

} // namespace LGE



