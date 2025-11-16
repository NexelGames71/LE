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

#include "LGE/core/GUIDRegistry.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <sstream>
#include <algorithm>

namespace LGE {

GUIDRegistry::GUIDRegistry() {
}

GUIDRegistry::~GUIDRegistry() {
}

GUID GUIDRegistry::RegisterAsset(const std::string& virtualPath) {
    std::string normalized = NormalizePath(virtualPath);
    
    // Check if already registered
    auto it = m_PathToGUID.find(normalized);
    if (it != m_PathToGUID.end()) {
        return it->second;
    }
    
    // Generate new GUID
    GUID guid = GUID::Generate();
    
    // Register in both maps
    m_PathToGUID[normalized] = guid;
    m_GUIDToPath[guid] = normalized;
    
    return guid;
}

GUID GUIDRegistry::GetOrCreateGUID(const std::string& virtualPath) {
    return RegisterAsset(virtualPath);
}

std::string GUIDRegistry::GetPathFromGUID(const GUID& guid) {
    auto it = m_GUIDToPath.find(guid);
    if (it != m_GUIDToPath.end()) {
        return it->second;
    }
    return "";
}

GUID GUIDRegistry::GetGUIDFromPath(const std::string& path) {
    std::string normalized = NormalizePath(path);
    auto it = m_PathToGUID.find(normalized);
    if (it != m_PathToGUID.end()) {
        return it->second;
    }
    return GUID::Invalid();
}

bool GUIDRegistry::UpdatePath(const GUID& guid, const std::string& newPath) {
    auto it = m_GUIDToPath.find(guid);
    if (it == m_GUIDToPath.end()) {
        return false;
    }
    
    std::string oldPath = it->second;
    std::string normalized = NormalizePath(newPath);
    
    // Remove old path entry
    m_PathToGUID.erase(oldPath);
    
    // Update both maps
    m_GUIDToPath[guid] = normalized;
    m_PathToGUID[normalized] = guid;
    
    return true;
}

bool GUIDRegistry::RemoveAsset(const GUID& guid) {
    auto it = m_GUIDToPath.find(guid);
    if (it == m_GUIDToPath.end()) {
        return false;
    }
    
    std::string path = it->second;
    m_GUIDToPath.erase(it);
    m_PathToGUID.erase(path);
    
    return true;
}

bool GUIDRegistry::RemoveAsset(const std::string& path) {
    std::string normalized = NormalizePath(path);
    auto it = m_PathToGUID.find(normalized);
    if (it == m_PathToGUID.end()) {
        return false;
    }
    
    GUID guid = it->second;
    m_PathToGUID.erase(it);
    m_GUIDToPath.erase(guid);
    
    return true;
}

bool GUIDRegistry::SaveToFile(const std::filesystem::path& path) {
    std::ostringstream json;
    json << "{\n";
    json << "  \"assets\": [\n";
    
    size_t index = 0;
    for (const auto& [guid, assetPath] : m_GUIDToPath) {
        json << "    {\n";
        json << "      \"guid\": \"" << guid.ToString() << "\",\n";
        json << "      \"path\": \"" << assetPath << "\"\n";
        json << "    }";
        if (index < m_GUIDToPath.size() - 1) {
            json << ",";
        }
        json << "\n";
        ++index;
    }
    
    json << "  ]\n";
    json << "}\n";
    
    if (!FileSystem::WriteFile(path.string(), json.str())) {
        Log::Error("Failed to save GUID registry to: " + path.string());
        return false;
    }
    
    Log::Info("Saved GUID registry with " + std::to_string(m_GUIDToPath.size()) + " assets to: " + path.string());
    return true;
}

bool GUIDRegistry::LoadFromFile(const std::filesystem::path& path) {
    if (!FileSystem::Exists(path.string())) {
        Log::Warn("GUID registry file not found: " + path.string());
        return false;
    }
    
    std::string json = FileSystem::ReadFile(path.string());
    if (json.empty()) {
        Log::Error("Failed to read GUID registry from: " + path.string());
        return false;
    }
    
    Clear();
    
    // Simple JSON parsing (basic implementation)
    // In production, use a proper JSON library
    
    size_t assetsPos = json.find("\"assets\"");
    if (assetsPos == std::string::npos) {
        Log::Error("Invalid GUID registry JSON format");
        return false;
    }
    
    size_t arrayStart = json.find('[', assetsPos);
    size_t arrayEnd = json.find(']', arrayStart);
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
        Log::Error("Invalid GUID registry JSON format: missing assets array");
        return false;
    }
    
    std::string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
    
    // Parse each asset entry
    size_t pos = 0;
    while ((pos = arrayContent.find('{', pos)) != std::string::npos) {
        size_t entryEnd = arrayContent.find('}', pos);
        if (entryEnd == std::string::npos) break;
        
        std::string entry = arrayContent.substr(pos, entryEnd - pos + 1);
        
        // Extract GUID
        size_t guidPos = entry.find("\"guid\"");
        size_t pathPos = entry.find("\"path\"");
        
        if (guidPos != std::string::npos && pathPos != std::string::npos) {
            // Parse GUID
            size_t guidColon = entry.find(':', guidPos);
            size_t guidQuoteStart = entry.find('"', guidColon);
            size_t guidQuoteEnd = entry.find('"', guidQuoteStart + 1);
            
            // Parse path
            size_t pathColon = entry.find(':', pathPos);
            size_t pathQuoteStart = entry.find('"', pathColon);
            size_t pathQuoteEnd = entry.find('"', pathQuoteStart + 1);
            
            if (guidQuoteStart != std::string::npos && guidQuoteEnd != std::string::npos &&
                pathQuoteStart != std::string::npos && pathQuoteEnd != std::string::npos) {
                
                std::string guidStr = entry.substr(guidQuoteStart + 1, guidQuoteEnd - guidQuoteStart - 1);
                std::string assetPath = entry.substr(pathQuoteStart + 1, pathQuoteEnd - pathQuoteStart - 1);
                
                GUID guid = GUID::FromString(guidStr);
                if (guid.IsValid()) {
                    std::string normalized = NormalizePath(assetPath);
                    m_GUIDToPath[guid] = normalized;
                    m_PathToGUID[normalized] = guid;
                }
            }
        }
        
        pos = entryEnd + 1;
    }
    
    Log::Info("Loaded GUID registry with " + std::to_string(m_GUIDToPath.size()) + " assets from: " + path.string());
    return true;
}

void GUIDRegistry::Clear() {
    m_GUIDToPath.clear();
    m_PathToGUID.clear();
}

std::string GUIDRegistry::NormalizePath(const std::string& path) const {
    std::string normalized = path;
    
    // Replace backslashes with forward slashes
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    
    // Ensure it starts with /
    if (normalized.empty() || normalized[0] != '/') {
        normalized = "/" + normalized;
    }
    
    // Remove trailing slash (except for root)
    if (normalized.length() > 1 && normalized.back() == '/') {
        normalized.pop_back();
    }
    
    return normalized;
}

} // namespace LGE

