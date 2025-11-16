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

#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/GUID.h"
#include "LGE/core/assets/AssetMetadata.h"
#include "LGE/rendering/Texture.h"
#include "LGE/core/filesystem/VirtualFileSystem.h"
#include <filesystem>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>

namespace LGE {

class ThumbnailGenerator {
private:
    AssetRegistry* m_Registry;
    VirtualFileSystem* m_VFS;
    std::filesystem::path m_CacheDirectory;
    std::unordered_map<GUID, std::shared_ptr<Texture>> m_Cache;
    std::mutex m_CacheMutex;
    
    // Background thread for generation
    std::thread m_GeneratorThread;
    std::queue<GUID> m_GenerationQueue;
    std::mutex m_QueueMutex;
    std::condition_variable m_QueueCV;
    std::atomic<bool> m_IsRunning;
    
    void GeneratorThreadFunc();
    std::shared_ptr<Texture> GenerateThumbnailForType(const AssetMetadata& metadata, int size = 128);
    std::shared_ptr<Texture> LoadCachedThumbnail(const GUID& guid);
    void SaveCachedThumbnail(const GUID& guid, std::shared_ptr<Texture> thumbnail);

public:
    ThumbnailGenerator(AssetRegistry* registry, VirtualFileSystem* vfs, const std::filesystem::path& cacheDir);
    ~ThumbnailGenerator();
    
    // Request thumbnail (async)
    void RequestThumbnail(const GUID& guid);
    
    // Get thumbnail (returns placeholder if not ready)
    std::shared_ptr<Texture> GetThumbnail(const GUID& guid);
    
    // Check if thumbnail is ready
    bool IsThumbnailReady(const GUID& guid);
    
    // Regenerate thumbnail
    void RegenerateThumbnail(const GUID& guid);
    
    // Clear cache
    void ClearCache();
    
    // Callbacks
    std::function<void(const GUID&)> onThumbnailReady;
    
    // Get placeholder texture
    std::shared_ptr<Texture> GetPlaceholderTexture();
};

// Type-specific thumbnail generators
class TextureThumbnailGenerator {
public:
    static std::shared_ptr<Texture> Generate(const std::filesystem::path& path, int size = 128);
};

class ModelThumbnailGenerator {
public:
    static std::shared_ptr<Texture> Generate(const std::filesystem::path& path, int size = 128);
};

} // namespace LGE



