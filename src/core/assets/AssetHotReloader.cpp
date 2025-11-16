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

#include "LGE/core/assets/AssetHotReloader.h"
#include "LGE/core/Log.h"
#include <algorithm>

namespace LGE {

AssetHotReloader::AssetHotReloader(AssetRegistry* registry, VirtualFileSystem* vfs)
    : m_Registry(registry)
    , m_VFS(vfs)
{
    m_Debouncer = std::make_unique<FileChangeDebouncer>(std::chrono::milliseconds(500));
    m_Debouncer->SetBatchCallback([this](const std::vector<FileChangeEvent>& events) {
        OnFilesChanged(events);
    });
}

AssetHotReloader::~AssetHotReloader() {
    Disable();
}

void AssetHotReloader::Enable(const std::filesystem::path& assetsDirectory) {
    if (m_Watcher && m_Watcher->IsWatching()) {
        Disable();
    }
    
    m_Watcher = FileSystemWatcher::Create();
    if (!m_Watcher) {
        Log::Error("Failed to create file system watcher");
        return;
    }
    
    m_Watcher->SetCallback([this](const FileChangeEvent& event) {
        m_Debouncer->AddEvent(event);
    });
    
    if (m_Watcher->StartWatching(assetsDirectory)) {
        Log::Info("Asset hot-reloading enabled for: " + assetsDirectory.string());
    } else {
        Log::Error("Failed to start watching directory: " + assetsDirectory.string());
        m_Watcher.reset();
    }
}

void AssetHotReloader::Disable() {
    if (m_Watcher) {
        m_Watcher->StopWatching();
        m_Watcher.reset();
    }
    
    if (m_Debouncer) {
        m_Debouncer->Flush();
    }
    
    Log::Info("Asset hot-reloading disabled");
}

void AssetHotReloader::RegisterReloadCallback(std::function<void(const GUID&, AssetType)> callback) {
    m_ReloadCallbacks.push_back(callback);
}

void AssetHotReloader::UnregisterReloadCallback(std::function<void(const GUID&, AssetType)> callback) {
    // Note: This is a simplified version. In practice, you'd want to store function IDs
    m_ReloadCallbacks.erase(
        std::remove_if(m_ReloadCallbacks.begin(), m_ReloadCallbacks.end(),
            [&callback](const std::function<void(const GUID&, AssetType)>& cb) {
                // Compare function targets (simplified - in practice use function IDs)
                return false; // For now, just keep all callbacks
            }),
        m_ReloadCallbacks.end()
    );
}

bool AssetHotReloader::IsEnabled() const {
    return m_Watcher && m_Watcher->IsWatching();
}

void AssetHotReloader::OnFilesChanged(const std::vector<FileChangeEvent>& events) {
    for (const auto& event : events) {
        // Get virtual path
        std::string virtualPath = m_VFS ? m_VFS->GetVirtualPath(event.path) : event.path.string();
        
        // Find asset by path
        AssetMetadata* asset = m_Registry->GetAssetByPath(virtualPath);
        if (!asset) {
            // New file - could trigger asset scanner
            continue;
        }
        
        GUID guid = asset->guid;
        AssetType type = asset->type;
        
        switch (event.type) {
            case FileChangeType::Added:
            case FileChangeType::Modified:
                ReloadAsset(guid);
                break;
                
            case FileChangeType::Removed:
                // Asset was deleted - notify callbacks
                for (auto& callback : m_ReloadCallbacks) {
                    callback(guid, type);
                }
                break;
                
            case FileChangeType::Renamed:
                // Update path in registry
                if (!event.oldPath.empty()) {
                    std::string oldVirtualPath = m_VFS ? m_VFS->GetVirtualPath(event.oldPath) : event.oldPath.string();
                    AssetMetadata* oldAsset = m_Registry->GetAssetByPath(oldVirtualPath);
                    if (oldAsset) {
                        oldAsset->virtualPath = virtualPath;
                        m_Registry->UpdateAsset(guid, *oldAsset);
                    }
                }
                ReloadAsset(guid);
                break;
        }
    }
}

void AssetHotReloader::ReloadAsset(const GUID& guid) {
    AssetMetadata* asset = m_Registry->GetAsset(guid);
    if (!asset) {
        return;
    }
    
    // Update file metadata
    std::filesystem::path physicalPath = m_VFS ? m_VFS->ResolveVirtualPath(asset->virtualPath) : asset->virtualPath;
    if (std::filesystem::exists(physicalPath)) {
        auto lastWriteTime = std::filesystem::last_write_time(physicalPath);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            lastWriteTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        auto timeT = std::chrono::system_clock::to_time_t(sctp);
        asset->lastModified = timeT;
        asset->fileSize = std::filesystem::file_size(physicalPath);
        m_Registry->UpdateAsset(guid, *asset);
    }
    
    // Notify all registered callbacks
    for (auto& callback : m_ReloadCallbacks) {
        callback(guid, asset->type);
    }
    
    Log::Info("Reloaded asset: " + asset->name + " (" + guid.ToString() + ")");
}

} // namespace LGE

