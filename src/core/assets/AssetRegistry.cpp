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

#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <algorithm>
#include <sstream>
#include <cctype>

namespace LGE {

AssetRegistry::AssetRegistry()
    : m_IsDirty(false)
{
}

AssetRegistry::~AssetRegistry() {
    if (m_IsDirty) {
        SaveToCache();
    }
}

bool AssetRegistry::RegisterAsset(const AssetMetadata& metadata) {
    if (!metadata.guid.IsValid()) {
        Log::Error("Cannot register asset with invalid GUID");
        return false;
    }
    
    // Check if already registered
    if (m_Assets.find(metadata.guid) != m_Assets.end()) {
        Log::Warn("Asset already registered: " + metadata.guid.ToString());
        return UpdateAsset(metadata.guid, metadata);
    }
    
    // Add to main map
    m_Assets[metadata.guid] = metadata;
    
    // Add to indices
    AddToIndices(metadata.guid, metadata);
    
    m_IsDirty = true;
    return true;
}

bool AssetRegistry::UnregisterAsset(const GUID& guid) {
    auto it = m_Assets.find(guid);
    if (it == m_Assets.end()) {
        return false;
    }
    
    AssetMetadata metadata = it->second;
    RemoveFromIndices(guid, metadata);
    m_Assets.erase(it);
    
    m_IsDirty = true;
    return true;
}

bool AssetRegistry::UpdateAsset(const GUID& guid, const AssetMetadata& metadata) {
    auto it = m_Assets.find(guid);
    if (it == m_Assets.end()) {
        return RegisterAsset(metadata);
    }
    
    AssetMetadata oldMetadata = it->second;
    it->second = metadata;
    
    UpdateIndices(guid, oldMetadata, metadata);
    
    m_IsDirty = true;
    return true;
}

AssetMetadata* AssetRegistry::GetAsset(const GUID& guid) {
    auto it = m_Assets.find(guid);
    if (it != m_Assets.end()) {
        return &it->second;
    }
    return nullptr;
}

const AssetMetadata* AssetRegistry::GetAsset(const GUID& guid) const {
    auto it = m_Assets.find(guid);
    if (it != m_Assets.end()) {
        return &it->second;
    }
    return nullptr;
}

AssetMetadata* AssetRegistry::GetAssetByPath(const std::string& virtualPath) {
    auto it = m_PathIndex.find(virtualPath);
    if (it != m_PathIndex.end()) {
        return GetAsset(it->second);
    }
    return nullptr;
}

const AssetMetadata* AssetRegistry::GetAssetByPath(const std::string& virtualPath) const {
    auto it = m_PathIndex.find(virtualPath);
    if (it != m_PathIndex.end()) {
        return GetAsset(it->second);
    }
    return nullptr;
}

std::vector<AssetMetadata*> AssetRegistry::GetAssetsByType(AssetType type) {
    std::vector<AssetMetadata*> result;
    auto range = m_TypeIndex.equal_range(type);
    for (auto it = range.first; it != range.second; ++it) {
        AssetMetadata* asset = GetAsset(it->second);
        if (asset) {
            result.push_back(asset);
        }
    }
    return result;
}

std::vector<const AssetMetadata*> AssetRegistry::GetAssetsByType(AssetType type) const {
    std::vector<const AssetMetadata*> result;
    auto range = m_TypeIndex.equal_range(type);
    for (auto it = range.first; it != range.second; ++it) {
        const AssetMetadata* asset = GetAsset(it->second);
        if (asset) {
            result.push_back(asset);
        }
    }
    return result;
}

std::vector<AssetMetadata*> AssetRegistry::GetAssetsByTag(const std::string& tag) {
    std::vector<AssetMetadata*> result;
    auto range = m_TagIndex.equal_range(tag);
    for (auto it = range.first; it != range.second; ++it) {
        AssetMetadata* asset = GetAsset(it->second);
        if (asset) {
            result.push_back(asset);
        }
    }
    return result;
}

std::vector<const AssetMetadata*> AssetRegistry::GetAssetsByTag(const std::string& tag) const {
    std::vector<const AssetMetadata*> result;
    auto range = m_TagIndex.equal_range(tag);
    for (auto it = range.first; it != range.second; ++it) {
        const AssetMetadata* asset = GetAsset(it->second);
        if (asset) {
            result.push_back(asset);
        }
    }
    return result;
}

std::vector<AssetMetadata*> AssetRegistry::SearchAssets(const std::string& query) {
    std::vector<AssetMetadata*> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (auto& [guid, metadata] : m_Assets) {
        // Search in name
        std::string lowerName = metadata.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        if (lowerName.find(lowerQuery) != std::string::npos) {
            result.push_back(&metadata);
            continue;
        }
        
        // Search in path
        std::string lowerPath = metadata.virtualPath;
        std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
        if (lowerPath.find(lowerQuery) != std::string::npos) {
            result.push_back(&metadata);
            continue;
        }
        
        // Search in tags
        for (const auto& tag : metadata.tags) {
            std::string lowerTag = tag;
            std::transform(lowerTag.begin(), lowerTag.end(), lowerTag.begin(), ::tolower);
            if (lowerTag.find(lowerQuery) != std::string::npos) {
                result.push_back(&metadata);
                break;
            }
        }
    }
    
    return result;
}

std::vector<const AssetMetadata*> AssetRegistry::SearchAssets(const std::string& query) const {
    std::vector<const AssetMetadata*> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& [guid, metadata] : m_Assets) {
        // Search in name
        std::string lowerName = metadata.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        if (lowerName.find(lowerQuery) != std::string::npos) {
            result.push_back(&metadata);
            continue;
        }
        
        // Search in path
        std::string lowerPath = metadata.virtualPath;
        std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
        if (lowerPath.find(lowerQuery) != std::string::npos) {
            result.push_back(&metadata);
            continue;
        }
        
        // Search in tags
        for (const auto& tag : metadata.tags) {
            std::string lowerTag = tag;
            std::transform(lowerTag.begin(), lowerTag.end(), lowerTag.begin(), ::tolower);
            if (lowerTag.find(lowerQuery) != std::string::npos) {
                result.push_back(&metadata);
                break;
            }
        }
    }
    
    return result;
}

std::vector<GUID> AssetRegistry::GetDependencies(const GUID& guid) {
    const AssetMetadata* asset = GetAsset(guid);
    if (asset) {
        return asset->dependencies;
    }
    return {};
}

std::vector<GUID> AssetRegistry::GetDependents(const GUID& guid) {
    std::vector<GUID> dependents;
    
    for (const auto& [assetGuid, metadata] : m_Assets) {
        for (const auto& dep : metadata.dependencies) {
            if (dep == guid) {
                dependents.push_back(assetGuid);
                break;
            }
        }
    }
    
    return dependents;
}

bool AssetRegistry::SaveToCache() {
    if (m_CacheFilePath.empty()) {
        Log::Warn("Cannot save asset registry: cache file path not set");
        return false;
    }
    
    std::ostringstream json;
    json << "{\n";
    json << "  \"assets\": [\n";
    
    size_t index = 0;
    for (const auto& [guid, metadata] : m_Assets) {
        json << "    " << metadata.ToJson();
        if (index < m_Assets.size() - 1) {
            json << ",";
        }
        json << "\n";
        ++index;
    }
    
    json << "  ]\n";
    json << "}\n";
    
    if (!FileSystem::WriteFile(m_CacheFilePath.string(), json.str())) {
        Log::Error("Failed to save asset registry cache");
        return false;
    }
    
    m_IsDirty = false;
    Log::Info("Saved asset registry cache with " + std::to_string(m_Assets.size()) + " assets");
    return true;
}

bool AssetRegistry::LoadFromCache() {
    if (m_CacheFilePath.empty() || !FileSystem::Exists(m_CacheFilePath.string())) {
        Log::Warn("Asset registry cache file not found: " + m_CacheFilePath.string());
        return false;
    }
    
    std::string json = FileSystem::ReadFile(m_CacheFilePath.string());
    if (json.empty()) {
        Log::Error("Failed to read asset registry cache");
        return false;
    }
    
    Clear();
    
    // Parse JSON
    size_t assetsPos = json.find("\"assets\"");
    if (assetsPos == std::string::npos) {
        Log::Error("Invalid asset registry cache format");
        return false;
    }
    
    size_t arrayStart = json.find('[', assetsPos);
    size_t arrayEnd = json.find(']', arrayStart);
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
        Log::Error("Invalid asset registry cache format: missing assets array");
        return false;
    }
    
    // Parse each asset (simplified - assumes each asset is a complete JSON object)
    size_t pos = arrayStart + 1;
    int braceDepth = 0;
    size_t objStart = 0;
    
    while (pos < arrayEnd) {
        if (json[pos] == '{') {
            if (braceDepth == 0) {
                objStart = pos;
            }
            ++braceDepth;
        } else if (json[pos] == '}') {
            --braceDepth;
            if (braceDepth == 0) {
                // Extract complete JSON object
                std::string assetJson = json.substr(objStart, pos - objStart + 1);
                AssetMetadata metadata = AssetMetadata::FromJson(assetJson);
                if (metadata.guid.IsValid()) {
                    RegisterAsset(metadata);
                }
            }
        }
        ++pos;
    }
    
    m_IsDirty = false;
    Log::Info("Loaded asset registry cache with " + std::to_string(m_Assets.size()) + " assets");
    return true;
}

size_t AssetRegistry::GetAssetCountByType(AssetType type) const {
    return m_TypeIndex.count(type);
}

void AssetRegistry::Clear() {
    m_Assets.clear();
    m_PathIndex.clear();
    m_TypeIndex.clear();
    m_TagIndex.clear();
    m_IsDirty = false;
}

void AssetRegistry::AddToIndices(const GUID& guid, const AssetMetadata& metadata) {
    // Path index
    m_PathIndex[metadata.virtualPath] = guid;
    
    // Type index
    m_TypeIndex.insert({metadata.type, guid});
    
    // Tag index
    for (const auto& tag : metadata.tags) {
        m_TagIndex.insert({tag, guid});
    }
}

void AssetRegistry::RemoveFromIndices(const GUID& guid, const AssetMetadata& metadata) {
    // Path index
    m_PathIndex.erase(metadata.virtualPath);
    
    // Type index
    auto typeRange = m_TypeIndex.equal_range(metadata.type);
    for (auto it = typeRange.first; it != typeRange.second; ) {
        if (it->second == guid) {
            it = m_TypeIndex.erase(it);
        } else {
            ++it;
        }
    }
    
    // Tag index
    for (const auto& tag : metadata.tags) {
        auto tagRange = m_TagIndex.equal_range(tag);
        for (auto it = tagRange.first; it != tagRange.second; ) {
            if (it->second == guid) {
                it = m_TagIndex.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void AssetRegistry::UpdateIndices(const GUID& guid, const AssetMetadata& oldMetadata, const AssetMetadata& newMetadata) {
    RemoveFromIndices(guid, oldMetadata);
    AddToIndices(guid, newMetadata);
}

} // namespace LGE



