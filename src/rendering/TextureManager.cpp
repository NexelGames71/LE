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

#include "LGE/rendering/TextureManager.h"
#include "LGE/core/Log.h"
#include <algorithm>
#include <filesystem>

namespace LGE {

TextureManager::TextureManager() {
}

TextureManager::~TextureManager() {
    Clear();
}

std::string TextureManager::NormalizePath(const std::string& filepath) const {
    try {
        // Convert to absolute path and normalize
        std::filesystem::path path(filepath);
        if (path.is_relative()) {
            // If relative, make it relative to current working directory
            path = std::filesystem::absolute(path);
        }
        // Normalize separators and case (on Windows)
        std::string normalized = path.string();
        std::replace(normalized.begin(), normalized.end(), '\\', '/');
        return normalized;
    } catch (...) {
        // Fallback: just use the original path
        return filepath;
    }
}

std::shared_ptr<Texture> TextureManager::GetOrLoad(const std::string& filepath) {
    std::string normalized = NormalizePath(filepath);
    
    // Check cache first
    auto it = m_TextureCache.find(normalized);
    if (it != m_TextureCache.end()) {
        return it->second;
    }
    
    // Load with default spec
    TextureSpec spec;
    spec.filepath = normalized;
    return Load(spec);
}

std::shared_ptr<Texture> TextureManager::GetOrLoad(const TextureSpec& spec) {
    std::string normalized = NormalizePath(spec.filepath);
    
    // Check cache first
    auto it = m_TextureCache.find(normalized);
    if (it != m_TextureCache.end()) {
        return it->second;
    }
    
    // Load with provided spec
    TextureSpec normalizedSpec = spec;
    normalizedSpec.filepath = normalized;
    return Load(normalizedSpec);
}

std::shared_ptr<Texture> TextureManager::Load(const TextureSpec& spec) {
    std::string normalized = NormalizePath(spec.filepath);
    
    // Check cache again (in case of race condition)
    auto it = m_TextureCache.find(normalized);
    if (it != m_TextureCache.end()) {
        return it->second;
    }
    
    // Create new texture
    auto texture = std::make_shared<Texture>();
    if (!texture->Load(spec)) {
        Log::Error("Failed to load texture: " + normalized);
        return nullptr;
    }
    
    // Cache it
    m_TextureCache[normalized] = texture;
    Log::Info("Loaded and cached texture: " + normalized);
    
    return texture;
}

std::shared_ptr<Texture> TextureManager::Get(const std::string& filepath) const {
    std::string normalized = NormalizePath(filepath);
    auto it = m_TextureCache.find(normalized);
    if (it != m_TextureCache.end()) {
        return it->second;
    }
    return nullptr;
}

bool TextureManager::IsCached(const std::string& filepath) const {
    std::string normalized = NormalizePath(filepath);
    return m_TextureCache.find(normalized) != m_TextureCache.end();
}

void TextureManager::Unload(const std::string& filepath) {
    std::string normalized = NormalizePath(filepath);
    auto it = m_TextureCache.find(normalized);
    if (it != m_TextureCache.end()) {
        m_TextureCache.erase(it);
        Log::Info("Unloaded texture from cache: " + normalized);
    }
}

void TextureManager::Clear() {
    m_TextureCache.clear();
    Log::Info("Cleared texture cache");
}

} // namespace LGE

