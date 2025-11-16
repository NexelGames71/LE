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
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <ctime>

namespace LGE {

class AssetRegistryCache {
private:
    struct CacheEntry {
        AssetMetadata metadata;
        std::time_t cacheTime;
        bool isDirty;
        
        CacheEntry() : cacheTime(0), isDirty(false) {}
    };
    
    std::filesystem::path m_CacheFile;
    std::unordered_map<GUID, CacheEntry> m_Cache;
    
    // Dirty tracking
    std::unordered_set<GUID> m_DirtyAssets;
    std::atomic<bool> m_NeedsSave{false};
    
    // Background save thread
    std::thread m_SaveThread;
    std::mutex m_SaveMutex;
    std::condition_variable m_SaveCV;
    std::atomic<bool> m_IsRunning{false};
    
    void SaveThreadFunc();

public:
    AssetRegistryCache(const std::filesystem::path& cacheFilePath);
    ~AssetRegistryCache();
    
    // Load entire cache
    bool LoadCache();
    
    // Save cache (async)
    void SaveCache();
    
    // Save cache (blocking)
    void SaveCacheBlocking();
    
    // Mark asset as dirty
    void MarkDirty(const GUID& guid);
    
    // Update cache entry
    void UpdateEntry(const GUID& guid, const AssetMetadata& metadata);
    
    // Get cache entry
    const AssetMetadata* GetEntry(const GUID& guid) const;
    
    // Get cache statistics
    struct CacheStats {
        size_t totalEntries;
        size_t dirtyEntries;
        uint64_t cacheFileSize;
        std::time_t lastSaveTime;
        
        CacheStats() : totalEntries(0), dirtyEntries(0), cacheFileSize(0), lastSaveTime(0) {}
    };
    CacheStats GetStats() const;
    
    // Clear cache
    void Clear();
};

} // namespace LGE

