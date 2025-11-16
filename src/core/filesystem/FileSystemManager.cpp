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

#include "LGE/core/filesystem/FileSystemManager.h"
#include "LGE/core/filesystem/VirtualFileSystem.h"
#include "LGE/core/GUIDRegistry.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/assets/AssetScanner.h"
#include "LGE/core/assets/DependencyGraph.h"
#include "LGE/core/assets/DependencyScanner.h"
#include "LGE/core/filesystem/FileSystemWatcher.h"
#include "LGE/core/assets/AssetHotReloader.h"
#include "LGE/core/assets/AssetSearchIndex.h"
#include "LGE/core/assets/AssetCollection.h"
#include "LGE/core/assets/SavedSearch.h"
#include "LGE/core/assets/AssetLoader.h"
#include "LGE/core/assets/ResourceManager.h"
#include "LGE/core/assets/AsyncAssetLoader.h"
#include "LGE/core/assets/AssetRegistryCache.h"
#include "LGE/core/assets/AssetValidator.h"
#include "LGE/core/Log.h"
#include <filesystem>

namespace LGE {

FileSystemManager::FileSystemManager()
    : m_IsInitialized(false)
{
}

FileSystemManager::~FileSystemManager() {
    Shutdown();
}

bool FileSystemManager::Initialize(std::shared_ptr<Project> project) {
    if (!project || !project->IsLoaded()) {
        Log::Error("Cannot initialize FileSystemManager with invalid project");
        return false;
    }
    
    // Shutdown previous initialization if any
    if (m_IsInitialized) {
        Shutdown();
    }
    
    m_Project = project;
    std::filesystem::path projectRoot = std::filesystem::path(project->GetProjectPath());
    
    // Initialize Virtual File System
    m_VFS = std::make_unique<VirtualFileSystem>();
    m_VFS->Initialize(projectRoot);
    
    // Initialize GUID Registry
    m_GUIDRegistry = std::make_unique<GUIDRegistry>();
    std::filesystem::path guidRegistryPath = projectRoot / "Intermediate" / "guid_registry.json";
    m_GUIDRegistry->LoadFromFile(guidRegistryPath);
    
    // Initialize Asset Registry
    m_AssetRegistry = std::make_unique<AssetRegistry>();
    std::filesystem::path assetCachePath = projectRoot / "Intermediate" / "AssetCache" / "asset_registry.json";
    m_AssetRegistry->SetCacheFilePath(assetCachePath);
    m_AssetRegistry->LoadFromCache();
    
    // Initialize Dependency Graph
    m_DependencyGraph = std::make_unique<DependencyGraph>(m_AssetRegistry.get());
    std::filesystem::path dependencyGraphPath = projectRoot / "Intermediate" / "dependency_graph.json";
    m_DependencyGraph->LoadFromFile(dependencyGraphPath);
    
    // Initialize Asset Scanner
    std::filesystem::path assetsPath = projectRoot / "Assets";
    m_AssetScanner = std::make_unique<AssetScanner>(
        m_AssetRegistry.get(), 
        m_GUIDRegistry.get(),
        m_VFS.get(), 
        assetsPath
    );
    
    // Initialize Dependency Scanner
    m_DependencyScanner = std::make_unique<DependencyScanner>(
        m_AssetRegistry.get(),
        m_DependencyGraph.get()
    );
    
    // Initialize Asset Search Index
    m_SearchIndex = std::make_unique<AssetSearchIndex>();
    m_SearchIndex->RebuildIndex(m_AssetRegistry.get());
    
    // Initialize Collection Manager
    m_CollectionManager = std::make_unique<CollectionManager>(
        m_AssetRegistry.get(),
        m_SearchIndex.get()
    );
    std::filesystem::path collectionsPath = projectRoot / "Intermediate" / "collections.json";
    m_CollectionManager->SetStorageFile(collectionsPath);
    m_CollectionManager->LoadFromFile();
    
    // Initialize Saved Search Manager
    m_SavedSearchManager = std::make_unique<SavedSearchManager>();
    std::filesystem::path savedSearchesPath = projectRoot / "Intermediate" / "saved_searches.json";
    m_SavedSearchManager->SetStorageFile(savedSearchesPath);
    m_SavedSearchManager->LoadFromFile();
    
    // Initialize Asset Loader
    m_AssetLoader = std::make_unique<AssetLoader>(m_AssetRegistry.get());
    
    // Initialize Resource Manager
    m_ResourceManager = std::make_unique<ResourceManager>(
        m_AssetLoader.get(),
        m_AssetRegistry.get()
    );
    
    // Initialize Async Asset Loader
    m_AsyncAssetLoader = std::make_unique<AsyncAssetLoader>(m_AssetLoader.get(), 4);
    
    // Initialize Asset Registry Cache
    std::filesystem::path cachePath = projectRoot / "Intermediate" / "AssetCache" / "registry_cache.json";
    m_RegistryCache = std::make_unique<AssetRegistryCache>(cachePath);
    m_RegistryCache->LoadCache();
    
    // Initialize Asset Validator
    m_AssetValidator = std::make_unique<AssetValidator>(
        m_AssetRegistry.get(),
        m_DependencyGraph.get()
    );
    
    // Initialize Hot Reloader
    m_HotReloader = std::make_unique<AssetHotReloader>(
        m_AssetRegistry.get(),
        m_VFS.get()
    );
    
    m_IsInitialized = true;
    Log::Info("FileSystemManager initialized for project: " + project->GetInfo().name);
    
    return true;
}

void FileSystemManager::Shutdown() {
    if (!m_IsInitialized) {
        return;
    }
    
    // Save all persistent data
    if (m_GUIDRegistry && m_Project) {
        std::filesystem::path guidRegistryPath = std::filesystem::path(m_Project->GetProjectPath()) / "Intermediate" / "guid_registry.json";
        m_GUIDRegistry->SaveToFile(guidRegistryPath);
    }
    
    if (m_AssetRegistry) {
        m_AssetRegistry->SaveToCache();
    }
    
    if (m_DependencyGraph && m_Project) {
        std::filesystem::path dependencyGraphPath = std::filesystem::path(m_Project->GetProjectPath()) / "Intermediate" / "dependency_graph.json";
        m_DependencyGraph->SaveToFile(dependencyGraphPath);
    }
    
    if (m_CollectionManager) {
        m_CollectionManager->SaveToFile();
    }
    
    if (m_SavedSearchManager) {
        m_SavedSearchManager->SaveToFile();
    }
    
    if (m_RegistryCache) {
        m_RegistryCache->SaveCacheBlocking();
    }
    
    // Disable hot reloading
    if (m_HotReloader) {
        m_HotReloader->Disable();
    }
    
    // Stop async loader
    if (m_AsyncAssetLoader) {
        m_AsyncAssetLoader->Stop();
    }
    
    // Clear all systems
    m_AsyncAssetLoader.reset();
    m_ResourceManager.reset();
    m_AssetLoader.reset();
    m_RegistryCache.reset();
    m_AssetValidator.reset();
    m_HotReloader.reset();
    m_FileWatcher.reset();
    m_SavedSearchManager.reset();
    m_CollectionManager.reset();
    m_SearchIndex.reset();
    m_DependencyScanner.reset();
    m_DependencyGraph.reset();
    m_AssetScanner.reset();
    m_AssetRegistry.reset();
    m_GUIDRegistry.reset();
    m_VFS.reset();
    
    m_Project.reset();
    m_IsInitialized = false;
    
    Log::Info("FileSystemManager shut down");
}

void FileSystemManager::ScanAssets(std::function<void(int, int)> progressCallback) {
    if (!m_IsInitialized || !m_AssetScanner) {
        return;
    }
    
    Log::Info("Starting asset scan...");
    m_AssetScanner->ScanAll(progressCallback);
    
    // Rebuild search index after scan
    if (m_SearchIndex) {
        m_SearchIndex->RebuildIndex(m_AssetRegistry.get());
    }
    
    // Scan dependencies
    if (m_DependencyScanner) {
        m_DependencyScanner->ScanAll(progressCallback);
    }
    
    Log::Info("Asset scan complete");
}

void FileSystemManager::EnableHotReloading(bool enable) {
    if (!m_IsInitialized || !m_HotReloader) {
        return;
    }
    
    if (enable) {
        std::filesystem::path assetsPath = std::filesystem::path(m_Project->GetProjectPath()) / "Assets";
        m_HotReloader->Enable(assetsPath);
    } else {
        m_HotReloader->Disable();
    }
}

} // namespace LGE

