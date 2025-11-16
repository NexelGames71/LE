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
#include "LGE/core/assets/AssetMetadata.h"
#include "LGE/core/GUIDRegistry.h"
#include "LGE/core/filesystem/VirtualFileSystem.h"
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <ctime>

namespace LGE {

class AssetScanner {
private:
    AssetRegistry* m_Registry;
    GUIDRegistry* m_GUIDRegistry;
    VirtualFileSystem* m_VFS;
    std::filesystem::path m_AssetsRoot;
    
    // Track last scan state
    std::unordered_map<std::string, std::time_t> m_LastScanTimes;
    
    AssetType DetectAssetType(const std::filesystem::path& path);
    bool ShouldIgnoreFile(const std::filesystem::path& path);
    bool ShouldIgnoreDirectory(const std::filesystem::path& path);

public:
    AssetScanner(AssetRegistry* registry, GUIDRegistry* guidRegistry, VirtualFileSystem* vfs, const std::filesystem::path& root);
    
    // Full scan of all assets
    void ScanAll(std::function<void(int, int)> progressCallback = nullptr);
    
    // Incremental scan (only changed files)
    void ScanIncremental();
    
    // Scan specific directory
    void ScanDirectory(const std::filesystem::path& directory, bool recursive = true);
    
    // Process single file
    bool ProcessFile(const std::filesystem::path& filePath);
    
    // Set assets root
    void SetAssetsRoot(const std::filesystem::path& root) { m_AssetsRoot = root; }
};

} // namespace LGE



