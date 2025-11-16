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

#include "LGE/core/assets/AssetScanner.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <algorithm>
#include <cctype>
#include <filesystem>

namespace LGE {

AssetScanner::AssetScanner(AssetRegistry* registry, GUIDRegistry* guidRegistry, VirtualFileSystem* vfs, const std::filesystem::path& root)
    : m_Registry(registry)
    , m_GUIDRegistry(guidRegistry)
    , m_VFS(vfs)
    , m_AssetsRoot(root)
{
}

AssetType AssetScanner::DetectAssetType(const std::filesystem::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    static const std::unordered_map<std::string, AssetType> extensionMap = {
        {".png", AssetType::Texture},
        {".jpg", AssetType::Texture},
        {".jpeg", AssetType::Texture},
        {".tga", AssetType::Texture},
        {".dds", AssetType::Texture},
        {".hdr", AssetType::Texture},
        {".exr", AssetType::Texture},
        {".fbx", AssetType::Model},
        {".obj", AssetType::Model},
        {".gltf", AssetType::Model},
        {".glb", AssetType::Model},
        {".lmat", AssetType::Material},
        {".material", AssetType::Material},
        {".lshader", AssetType::Shader},
        {".vert", AssetType::Shader},
        {".frag", AssetType::Shader},
        {".glsl", AssetType::Shader},
        {".lua", AssetType::Script},
        {".cs", AssetType::Script},
        {".py", AssetType::Script},
        {".wav", AssetType::Audio},
        {".mp3", AssetType::Audio},
        {".ogg", AssetType::Audio},
        {".lscene", AssetType::Scene},
        {".scene", AssetType::Scene},
        {".prefab", AssetType::Prefab},
        {".anim", AssetType::Animation},
        {".ttf", AssetType::Font},
        {".otf", AssetType::Font}
    };
    
    auto it = extensionMap.find(ext);
    return (it != extensionMap.end()) ? it->second : AssetType::Unknown;
}

bool AssetScanner::ShouldIgnoreFile(const std::filesystem::path& path) {
    std::string filename = path.filename().string();
    
    // Ignore hidden files (starting with .)
    if (!filename.empty() && filename[0] == '.') {
        return true;
    }
    
    // Ignore common temporary/metadata files
    static const std::vector<std::string> ignorePatterns = {
        "~$",           // Temporary files
        ".tmp",
        ".bak",
        ".cache",
        ".meta",        // Unity-style metadata
        "Thumbs.db"     // Windows thumbnail cache
    };
    
    for (const auto& pattern : ignorePatterns) {
        if (filename.find(pattern) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

bool AssetScanner::ShouldIgnoreDirectory(const std::filesystem::path& path) {
    std::string dirname = path.filename().string();
    
    // Ignore hidden directories
    if (!dirname.empty() && dirname[0] == '.') {
        return true;
    }
    
    // Ignore common build/cache directories
    static const std::vector<std::string> ignoreDirs = {
        "Intermediate",
        "Binaries",
        "DerivedDataCache",
        ".vs",
        ".vscode",
        "node_modules"
    };
    
    for (const auto& ignoreDir : ignoreDirs) {
        if (dirname == ignoreDir) {
            return true;
        }
    }
    
    return false;
}

void AssetScanner::ScanAll(std::function<void(int, int)> progressCallback) {
    if (!std::filesystem::exists(m_AssetsRoot)) {
        Log::Warn("Assets root does not exist: " + m_AssetsRoot.string());
        return;
    }
    
    // Count total files first
    int totalFiles = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_AssetsRoot)) {
        if (entry.is_regular_file() && !ShouldIgnoreFile(entry.path())) {
            ++totalFiles;
        }
    }
    
    int processedFiles = 0;
    
    // Scan all files
    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_AssetsRoot)) {
        if (entry.is_regular_file() && !ShouldIgnoreFile(entry.path())) {
            ProcessFile(entry.path());
            ++processedFiles;
            
            if (progressCallback) {
                progressCallback(processedFiles, totalFiles);
            }
        }
    }
    
    // Update scan times
    m_LastScanTimes[m_AssetsRoot.string()] = std::time(nullptr);
    
    Log::Info("Scanned " + std::to_string(processedFiles) + " assets");
}

void AssetScanner::ScanIncremental() {
    if (!std::filesystem::exists(m_AssetsRoot)) {
        return;
    }
    
    int scannedCount = 0;
    int updatedCount = 0;
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_AssetsRoot)) {
        if (entry.is_regular_file() && !ShouldIgnoreFile(entry.path())) {
            std::string filePath = entry.path().string();
            
            // Check if file was modified since last scan
            auto lastWriteTime = std::filesystem::last_write_time(entry.path());
            auto lastWriteTimeT = std::chrono::duration_cast<std::chrono::seconds>(
                lastWriteTime.time_since_epoch()).count();
            
            auto it = m_LastScanTimes.find(filePath);
            bool needsUpdate = (it == m_LastScanTimes.end() || 
                               static_cast<std::time_t>(lastWriteTimeT) > it->second);
            
            if (needsUpdate) {
                ProcessFile(entry.path());
                m_LastScanTimes[filePath] = static_cast<std::time_t>(lastWriteTimeT);
                ++updatedCount;
            }
            ++scannedCount;
        }
    }
    
    Log::Info("Incremental scan: " + std::to_string(updatedCount) + " assets updated out of " + 
              std::to_string(scannedCount) + " scanned");
}

void AssetScanner::ScanDirectory(const std::filesystem::path& directory, bool recursive) {
    if (!std::filesystem::exists(directory)) {
        Log::Warn("Directory does not exist: " + directory.string());
        return;
    }
    
    if (recursive) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file() && !ShouldIgnoreFile(entry.path())) {
                ProcessFile(entry.path());
            }
        }
    } else {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && !ShouldIgnoreFile(entry.path())) {
                ProcessFile(entry.path());
            }
        }
    }
}

bool AssetScanner::ProcessFile(const std::filesystem::path& filePath) {
    if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath)) {
        return false;
    }
    
    // Detect asset type
    AssetType type = DetectAssetType(filePath);
    if (type == AssetType::Unknown) {
        return false; // Skip unknown file types
    }
    
    // Get virtual path
    std::string virtualPath = m_VFS ? m_VFS->GetVirtualPath(filePath) : filePath.string();
    if (virtualPath.empty()) {
        virtualPath = "/" + std::filesystem::relative(filePath, m_AssetsRoot).string();
        std::replace(virtualPath.begin(), virtualPath.end(), '\\', '/');
    }
    
    // Get or create GUID
    GUID guid = m_GUIDRegistry ? m_GUIDRegistry->GetOrCreateGUID(virtualPath) : GUID::Generate();
    
    // Check if asset already exists
    AssetMetadata* existing = m_Registry->GetAssetByPath(virtualPath);
    if (existing && existing->guid == guid) {
        // Update last modified time
        auto lastWriteTime = std::filesystem::last_write_time(filePath);
        auto lastWriteTimeT = std::chrono::duration_cast<std::chrono::seconds>(
            lastWriteTime.time_since_epoch()).count();
        
        if (static_cast<std::time_t>(lastWriteTimeT) > existing->lastModified) {
            existing->lastModified = static_cast<std::time_t>(lastWriteTimeT);
            existing->fileSize = std::filesystem::file_size(filePath);
            m_Registry->UpdateAsset(guid, *existing);
        }
        return true;
    }
    
    // Create new metadata
    AssetMetadata metadata;
    metadata.guid = guid;
    metadata.virtualPath = virtualPath;
    metadata.type = type;
    metadata.name = filePath.filename().string();
    metadata.fileSize = std::filesystem::file_size(filePath);
    
    auto lastWriteTime = std::filesystem::last_write_time(filePath);
    auto lastWriteTimeT = std::chrono::duration_cast<std::chrono::seconds>(
        lastWriteTime.time_since_epoch()).count();
    metadata.lastModified = static_cast<std::time_t>(lastWriteTimeT);
    metadata.importDate = std::time(nullptr);
    
    // Set category based on parent directory
    std::filesystem::path parent = filePath.parent_path();
    if (parent != m_AssetsRoot) {
        metadata.category = parent.filename().string();
    }
    
    // Register asset
    if (m_Registry->RegisterAsset(metadata)) {
        return true;
    }
    
    return false;
}

} // namespace LGE



