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

#include "LGE/core/assets/ThumbnailGenerator.h"
#include "LGE/core/Log.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/rendering/Texture.h"
#include <algorithm>

// Include stb_image (already implemented in Texture.cpp)
#include "../../third_party/stb_image.h"

namespace LGE {

ThumbnailGenerator::ThumbnailGenerator(AssetRegistry* registry, VirtualFileSystem* vfs, const std::filesystem::path& cacheDir)
    : m_Registry(registry)
    , m_VFS(vfs)
    , m_CacheDirectory(cacheDir)
    , m_IsRunning(true)
{
    // Create cache directory if it doesn't exist
    if (!std::filesystem::exists(m_CacheDirectory)) {
        std::filesystem::create_directories(m_CacheDirectory);
    }
    
    // Start generator thread
    m_GeneratorThread = std::thread(&ThumbnailGenerator::GeneratorThreadFunc, this);
}

ThumbnailGenerator::~ThumbnailGenerator() {
    m_IsRunning = false;
    m_QueueCV.notify_all();
    
    if (m_GeneratorThread.joinable()) {
        m_GeneratorThread.join();
    }
}

void ThumbnailGenerator::RequestThumbnail(const GUID& guid) {
    // Check if already in cache
    {
        std::lock_guard<std::mutex> lock(m_CacheMutex);
        if (m_Cache.find(guid) != m_Cache.end()) {
            return; // Already cached
        }
    }
    
    // Check if already in queue
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        // Simple check - in production, would use a set to avoid duplicates
        m_GenerationQueue.push(guid);
    }
    
    m_QueueCV.notify_one();
}

std::shared_ptr<Texture> ThumbnailGenerator::GetThumbnail(const GUID& guid) {
    std::lock_guard<std::mutex> lock(m_CacheMutex);
    
    auto it = m_Cache.find(guid);
    if (it != m_Cache.end()) {
        return it->second;
    }
    
    // Return placeholder
    return GetPlaceholderTexture();
}

bool ThumbnailGenerator::IsThumbnailReady(const GUID& guid) {
    std::lock_guard<std::mutex> lock(m_CacheMutex);
    return m_Cache.find(guid) != m_Cache.end();
}

void ThumbnailGenerator::RegenerateThumbnail(const GUID& guid) {
    // Remove from cache
    {
        std::lock_guard<std::mutex> lock(m_CacheMutex);
        m_Cache.erase(guid);
    }
    
    // Request regeneration
    RequestThumbnail(guid);
}

void ThumbnailGenerator::ClearCache() {
    std::lock_guard<std::mutex> lock(m_CacheMutex);
    m_Cache.clear();
}

std::shared_ptr<Texture> ThumbnailGenerator::GetPlaceholderTexture() {
    // Create a simple placeholder texture (1x1 gray pixel)
    static std::shared_ptr<Texture> placeholder = nullptr;
    
    if (!placeholder) {
        placeholder = std::make_shared<Texture>();
        uint8_t grayData[4] = {128, 128, 128, 255};
        // Create texture from data (simplified - would need proper Texture creation)
    }
    
    return placeholder;
}

void ThumbnailGenerator::GeneratorThreadFunc() {
    while (m_IsRunning.load()) {
        GUID guid;
        bool hasWork = false;
        
        // Get next item from queue
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_QueueCV.wait(lock, [this] { return !m_GenerationQueue.empty() || !m_IsRunning.load(); });
            
            if (!m_GenerationQueue.empty()) {
                guid = m_GenerationQueue.front();
                m_GenerationQueue.pop();
                hasWork = true;
            }
        }
        
        if (!hasWork) continue;
        
        // Generate thumbnail
        if (!m_Registry) continue;
        
        const AssetMetadata* metadata = m_Registry->GetAsset(guid);
        if (!metadata) continue;
        
        std::shared_ptr<Texture> thumbnail = GenerateThumbnailForType(*metadata);
        
        if (thumbnail) {
            // Store in cache
            {
                std::lock_guard<std::mutex> lock(m_CacheMutex);
                m_Cache[guid] = thumbnail;
            }
            
            // Notify callback
            if (onThumbnailReady) {
                onThumbnailReady(guid);
            }
        }
    }
}

std::shared_ptr<Texture> ThumbnailGenerator::GenerateThumbnailForType(const AssetMetadata& metadata, int size) {
    std::filesystem::path assetPath = m_VFS ? m_VFS->ResolveVirtualPath(metadata.virtualPath) : metadata.virtualPath;
    
    switch (metadata.type) {
        case AssetType::Texture:
            return TextureThumbnailGenerator::Generate(assetPath, size);
        case AssetType::Model:
            return ModelThumbnailGenerator::Generate(assetPath, size);
        default:
            // Return placeholder for unsupported types
            return GetPlaceholderTexture();
    }
}

std::shared_ptr<Texture> ThumbnailGenerator::LoadCachedThumbnail(const GUID& guid) {
    std::filesystem::path cachePath = m_CacheDirectory / (guid.ToString() + ".png");
    
    if (!std::filesystem::exists(cachePath)) {
        return nullptr;
    }
    
    // Load cached thumbnail
    auto texture = std::make_shared<Texture>();
    // Load texture from file (simplified)
    return texture;
}

void ThumbnailGenerator::SaveCachedThumbnail(const GUID& guid, std::shared_ptr<Texture> thumbnail) {
    // Save thumbnail to cache directory
    // Implementation would save texture as PNG
}

// TextureThumbnailGenerator implementation
std::shared_ptr<Texture> TextureThumbnailGenerator::Generate(const std::filesystem::path& path, int size) {
    if (!std::filesystem::exists(path)) {
        return nullptr;
    }
    
    // Load image
    int width, height, channels;
    unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
    
    if (!data) {
        return nullptr;
    }
    
    // For now, create a simple texture from the loaded data
    // In production, would resize to thumbnail size
    auto texture = std::make_shared<Texture>();
    // Create texture from data (would need proper Texture::Create method)
    
    stbi_image_free(data);
    return texture;
}

// ModelThumbnailGenerator implementation
std::shared_ptr<Texture> ModelThumbnailGenerator::Generate(const std::filesystem::path& path, int size) {
    // Placeholder - would load model and render to offscreen buffer
    return nullptr;
}

} // namespace LGE



