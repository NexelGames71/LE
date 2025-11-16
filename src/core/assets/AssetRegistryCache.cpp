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

#include "LGE/core/assets/AssetRegistryCache.h"
#include "LGE/core/assets/AssetMetadata.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <sstream>
#include <chrono>

namespace LGE {

AssetRegistryCache::AssetRegistryCache(const std::filesystem::path& cacheFilePath)
    : m_CacheFile(cacheFilePath)
    , m_IsRunning(true)
{
    m_SaveThread = std::thread(&AssetRegistryCache::SaveThreadFunc, this);
}

AssetRegistryCache::~AssetRegistryCache() {
    m_IsRunning = false;
    m_SaveCV.notify_all();
    
    if (m_SaveThread.joinable()) {
        m_SaveThread.join();
    }
    
    // Final save
    SaveCacheBlocking();
}

void AssetRegistryCache::SaveThreadFunc() {
    while (m_IsRunning) {
        std::unique_lock<std::mutex> lock(m_SaveMutex);
        m_SaveCV.wait(lock, [this] { return m_NeedsSave || !m_IsRunning; });
        
        if (m_NeedsSave && m_IsRunning) {
            m_NeedsSave = false;
            lock.unlock();
            
            SaveCacheBlocking();
        }
    }
}

bool AssetRegistryCache::LoadCache() {
    if (!FileSystem::Exists(m_CacheFile.string())) {
        Log::Info("Cache file not found: " + m_CacheFile.string());
        return true; // Not an error
    }
    
    std::string jsonString = FileSystem::ReadFile(m_CacheFile.string());
    if (jsonString.empty()) {
        Log::Error("Failed to read cache file: " + m_CacheFile.string());
        return false;
    }
    
    Clear();
    
    // Parse JSON (simplified - would need full JSON parsing)
    // For now, this is a placeholder
    Log::Info("Cache loaded from: " + m_CacheFile.string());
    return true;
}

void AssetRegistryCache::SaveCache() {
    m_NeedsSave = true;
    m_SaveCV.notify_one();
}

void AssetRegistryCache::SaveCacheBlocking() {
    std::ostringstream json;
    json << "{\n";
    json << "  \"entries\": [\n";
    
    size_t index = 0;
    for (const auto& pair : m_Cache) {
        const CacheEntry& entry = pair.second;
        std::string metadataJson = entry.metadata.ToJson();
        
        // Indent metadata JSON
        std::istringstream iss(metadataJson);
        std::string line;
        while (std::getline(iss, line)) {
            json << "    " << line << "\n";
        }
        
        if (index < m_Cache.size() - 1) {
            json << "    ,\n";
        }
        index++;
    }
    
    json << "  ]\n";
    json << "}\n";
    
    if (FileSystem::WriteFile(m_CacheFile.string(), json.str())) {
        m_DirtyAssets.clear();
        Log::Info("Cache saved to: " + m_CacheFile.string());
    } else {
        Log::Error("Failed to save cache to: " + m_CacheFile.string());
    }
}

void AssetRegistryCache::MarkDirty(const GUID& guid) {
    m_DirtyAssets.insert(guid);
    auto it = m_Cache.find(guid);
    if (it != m_Cache.end()) {
        it->second.isDirty = true;
    }
}

void AssetRegistryCache::UpdateEntry(const GUID& guid, const AssetMetadata& metadata) {
    CacheEntry& entry = m_Cache[guid];
    entry.metadata = metadata;
    entry.cacheTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    entry.isDirty = true;
    m_DirtyAssets.insert(guid);
}

const AssetMetadata* AssetRegistryCache::GetEntry(const GUID& guid) const {
    auto it = m_Cache.find(guid);
    if (it != m_Cache.end()) {
        return &it->second.metadata;
    }
    return nullptr;
}

AssetRegistryCache::CacheStats AssetRegistryCache::GetStats() const {
    CacheStats stats;
    stats.totalEntries = m_Cache.size();
    stats.dirtyEntries = m_DirtyAssets.size();
    
    if (FileSystem::Exists(m_CacheFile.string())) {
        stats.cacheFileSize = std::filesystem::file_size(m_CacheFile);
    }
    
    // Find latest save time from cache entries
    std::time_t latestTime = 0;
    for (const auto& pair : m_Cache) {
        if (pair.second.cacheTime > latestTime) {
            latestTime = pair.second.cacheTime;
        }
    }
    stats.lastSaveTime = latestTime;
    
    return stats;
}

void AssetRegistryCache::Clear() {
    m_Cache.clear();
    m_DirtyAssets.clear();
    Log::Info("Asset registry cache cleared");
}

} // namespace LGE

