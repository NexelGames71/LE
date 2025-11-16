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
#include "LGE/core/filesystem/FileSystemWatcher.h"
#include "LGE/core/filesystem/FileChangeDebouncer.h"
#include "LGE/core/filesystem/VirtualFileSystem.h"
#include <memory>
#include <vector>
#include <functional>
#include <filesystem>

namespace LGE {

class AssetHotReloader {
private:
    AssetRegistry* m_Registry;
    VirtualFileSystem* m_VFS;
    std::unique_ptr<FileSystemWatcher> m_Watcher;
    std::unique_ptr<FileChangeDebouncer> m_Debouncer;
    
    void OnFilesChanged(const std::vector<FileChangeEvent>& events);
    void ReloadAsset(const GUID& guid);

public:
    AssetHotReloader(AssetRegistry* registry, VirtualFileSystem* vfs);
    ~AssetHotReloader();
    
    void Enable(const std::filesystem::path& assetsDirectory);
    void Disable();
    
    // Callbacks for systems that need to respond to asset changes
    std::vector<std::function<void(const GUID&, AssetType)>> m_ReloadCallbacks;
    void RegisterReloadCallback(std::function<void(const GUID&, AssetType)> callback);
    void UnregisterReloadCallback(std::function<void(const GUID&, AssetType)> callback);
    
    bool IsEnabled() const;
};

} // namespace LGE



