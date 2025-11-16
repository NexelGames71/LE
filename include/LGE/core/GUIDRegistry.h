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
#include <string>
#include <unordered_map>
#include <filesystem>

namespace LGE {

class GUIDRegistry {
private:
    std::unordered_map<GUID, std::string> m_GUIDToPath;
    std::unordered_map<std::string, GUID> m_PathToGUID;

public:
    GUIDRegistry();
    ~GUIDRegistry();
    
    // Register an asset and get/create its GUID
    GUID RegisterAsset(const std::string& virtualPath);
    
    // Get or create GUID for an asset
    GUID GetOrCreateGUID(const std::string& virtualPath);
    
    // Get path from GUID
    std::string GetPathFromGUID(const GUID& guid);
    
    // Get GUID from path
    GUID GetGUIDFromPath(const std::string& path);
    
    // Update path for existing GUID (e.g., when asset is moved)
    bool UpdatePath(const GUID& guid, const std::string& newPath);
    
    // Remove asset from registry
    bool RemoveAsset(const GUID& guid);
    bool RemoveAsset(const std::string& path);
    
    // Save registry to file
    bool SaveToFile(const std::filesystem::path& path);
    
    // Load registry from file
    bool LoadFromFile(const std::filesystem::path& path);
    
    // Clear all entries
    void Clear();
    
    // Get count of registered assets
    size_t GetAssetCount() const { return m_GUIDToPath.size(); }

private:
    // Helper methods
    std::string NormalizePath(const std::string& path) const;
};

} // namespace LGE

