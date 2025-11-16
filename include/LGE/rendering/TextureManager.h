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

#include "LGE/rendering/Texture.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace LGE {

// TextureManager - Handles texture caching and loading
class TextureManager {
public:
    TextureManager();
    ~TextureManager();

    // Get or load texture (cached)
    std::shared_ptr<Texture> GetOrLoad(const std::string& filepath);
    std::shared_ptr<Texture> GetOrLoad(const TextureSpec& spec);
    
    // Load texture with specification
    std::shared_ptr<Texture> Load(const TextureSpec& spec);
    
    // Get cached texture (returns nullptr if not found)
    std::shared_ptr<Texture> Get(const std::string& filepath) const;
    
    // Check if texture is cached
    bool IsCached(const std::string& filepath) const;
    
    // Remove texture from cache
    void Unload(const std::string& filepath);
    
    // Clear all cached textures
    void Clear();
    
    // Get cache statistics
    size_t GetCacheSize() const { return m_TextureCache.size(); }

private:
    // Normalize filepath for cache key (handles relative/absolute paths)
    std::string NormalizePath(const std::string& filepath) const;
    
    // Texture cache: filepath -> texture
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_TextureCache;
};

} // namespace LGE

