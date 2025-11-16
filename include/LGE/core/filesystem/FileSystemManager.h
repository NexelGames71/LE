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

#include "LGE/core/project/Project.h"
#include <memory>
#include <filesystem>
#include <functional>

namespace LGE {

// Forward declarations
class VirtualFileSystem;
class AssetRegistry;
class GUIDRegistry;
class AssetScanner;
class DependencyGraph;
class DependencyScanner;
class FileSystemWatcher;
class AssetHotReloader;
class AssetSearchIndex;
class CollectionManager;
class SavedSearchManager;
class AssetLoader;
class ResourceManager;
class AsyncAssetLoader;
class AssetRegistryCache;
class AssetValidator;

class FileSystemManager {
private:
    std::shared_ptr<Project> m_Project;
    
    // Core systems
    std::unique_ptr<VirtualFileSystem> m_VFS;
    std::unique_ptr<GUIDRegistry> m_GUIDRegistry;
    std::unique_ptr<AssetRegistry> m_AssetRegistry;
    std::unique_ptr<AssetScanner> m_AssetScanner;
    
    // Dependency management
    std::unique_ptr<DependencyGraph> m_DependencyGraph;
    std::unique_ptr<DependencyScanner> m_DependencyScanner;
    
    // File watching
    std::unique_ptr<FileSystemWatcher> m_FileWatcher;
    std::unique_ptr<AssetHotReloader> m_HotReloader;
    
    // Search and filtering
    std::unique_ptr<AssetSearchIndex> m_SearchIndex;
    std::unique_ptr<CollectionManager> m_CollectionManager;
    std::unique_ptr<SavedSearchManager> m_SavedSearchManager;
    
    // Asset loading
    std::unique_ptr<AssetLoader> m_AssetLoader;
    std::unique_ptr<ResourceManager> m_ResourceManager;
    std::unique_ptr<AsyncAssetLoader> m_AsyncAssetLoader;
    
    // Performance
    std::unique_ptr<AssetRegistryCache> m_RegistryCache;
    
    // Validation
    std::unique_ptr<AssetValidator> m_AssetValidator;
    
    bool m_IsInitialized;

public:
    FileSystemManager();
    ~FileSystemManager();
    
    // Initialize with a project
    bool Initialize(std::shared_ptr<Project> project);
    
    // Shutdown and cleanup
    void Shutdown();
    
    // Check if initialized
    bool IsInitialized() const { return m_IsInitialized; }
    
    // Get project
    std::shared_ptr<Project> GetProject() const { return m_Project; }
    
    // Get system accessors
    VirtualFileSystem* GetVFS() const { return m_VFS.get(); }
    AssetRegistry* GetAssetRegistry() const { return m_AssetRegistry.get(); }
    GUIDRegistry* GetGUIDRegistry() const { return m_GUIDRegistry.get(); }
    AssetScanner* GetAssetScanner() const { return m_AssetScanner.get(); }
    DependencyGraph* GetDependencyGraph() const { return m_DependencyGraph.get(); }
    DependencyScanner* GetDependencyScanner() const { return m_DependencyScanner.get(); }
    AssetHotReloader* GetHotReloader() const { return m_HotReloader.get(); }
    AssetSearchIndex* GetSearchIndex() const { return m_SearchIndex.get(); }
    CollectionManager* GetCollectionManager() const { return m_CollectionManager.get(); }
    SavedSearchManager* GetSavedSearchManager() const { return m_SavedSearchManager.get(); }
    AssetLoader* GetAssetLoader() const { return m_AssetLoader.get(); }
    ResourceManager* GetResourceManager() const { return m_ResourceManager.get(); }
    AsyncAssetLoader* GetAsyncAssetLoader() const { return m_AsyncAssetLoader.get(); }
    AssetValidator* GetAssetValidator() const { return m_AssetValidator.get(); }
    
    // Perform initial asset scan
    void ScanAssets(std::function<void(int, int)> progressCallback = nullptr);
    
    // Enable hot reloading
    void EnableHotReloading(bool enable = true);
};

} // namespace LGE

