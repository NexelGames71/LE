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

#include "LGE/core/assets/AssetDeleter.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/assets/DependencyGraph.h"
#include "LGE/core/filesystem/VirtualFileSystem.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <algorithm>

namespace LGE {

AssetDeleter::AssetDeleter(AssetRegistry* registry, DependencyGraph* graph, VirtualFileSystem* vfs)
    : m_Registry(registry)
    , m_Graph(graph)
    , m_VFS(vfs)
    , m_Validator(graph, registry)
{
    // Default trash path
    m_TrashPath = std::filesystem::current_path() / "Trash";
}

AssetDeleter::DeletionResult AssetDeleter::DeleteAssets(
    const std::vector<GUID>& assets, 
    DeletionMode mode)
{
    DeletionResult result;
    
    if (assets.empty()) {
        result.errorMessage = "No assets provided for deletion";
        return result;
    }
    
    // Validate deletion based on mode
    if (mode == DeletionMode::SafeOnly) {
        std::vector<AssetDeletionValidator::DeletionInfo> validation = m_Validator.ValidateDeletion(assets);
        if (!m_Validator.CanDeleteSafely(validation)) {
            result.errorMessage = "Cannot safely delete: assets are referenced by other assets";
            result.failedAssets = assets;
            return result;
        }
    }
    
    // Collect all assets to delete (including dependents for cascade mode)
    std::vector<GUID> assetsToDelete = assets;
    if (mode == DeletionMode::CascadeDelete) {
        for (const auto& asset : assets) {
            std::vector<GUID> dependents = m_Graph->GetAllDependents(asset, true);
            assetsToDelete.insert(assetsToDelete.end(), dependents.begin(), dependents.end());
        }
        // Remove duplicates
        std::sort(assetsToDelete.begin(), assetsToDelete.end());
        assetsToDelete.erase(std::unique(assetsToDelete.begin(), assetsToDelete.end()), assetsToDelete.end());
    }
    
    // Delete each asset
    for (const auto& asset : assetsToDelete) {
        if (mode == DeletionMode::BreakReferences) {
            BreakReferences(asset);
        }
        
        if (DeleteAssetFile(asset)) {
            // Remove from registry
            if (m_Registry) {
                m_Registry->UnregisterAsset(asset);
            }
            
            // Remove from dependency graph
            if (m_Graph) {
                m_Graph->RemoveDependencies(asset);
            }
            
            result.deletedAssets.push_back(asset);
        } else {
            result.failedAssets.push_back(asset);
        }
    }
    
    result.success = result.failedAssets.empty();
    if (!result.success) {
        result.errorMessage = "Failed to delete " + std::to_string(result.failedAssets.size()) + " asset(s)";
    }
    
    return result;
}

bool AssetDeleter::DeleteAssetFile(const GUID& asset) {
    if (!m_Registry || !m_VFS) {
        return false;
    }
    
    const AssetMetadata* metadata = m_Registry->GetAsset(asset);
    if (!metadata) {
        Log::Warn("Asset not found for deletion: " + asset.ToString());
        return false;
    }
    
    std::filesystem::path filePath = m_VFS->ResolveVirtualPath(metadata->virtualPath);
    
    try {
        if (std::filesystem::exists(filePath)) {
            std::filesystem::remove(filePath);
            
            // Also delete .meta file if it exists
            std::filesystem::path metaPath = filePath.string() + ".meta";
            if (std::filesystem::exists(metaPath)) {
                std::filesystem::remove(metaPath);
            }
            
            Log::Info("Deleted asset file: " + metadata->virtualPath);
            return true;
        } else {
            Log::Warn("Asset file does not exist: " + metadata->virtualPath);
            return true; // Consider it successful if file doesn't exist
        }
    } catch (const std::exception& e) {
        Log::Error("Failed to delete asset file: " + std::string(e.what()));
        return false;
    }
}

void AssetDeleter::BreakReferences(const GUID& asset) {
    if (!m_Graph) {
        return;
    }
    
    // Get all assets that depend on this one
    std::vector<GUID> dependents = m_Graph->GetDependents(asset);
    
    for (const auto& dependent : dependents) {
        // Remove the dependency relationship
        m_Graph->RemoveDependency(dependent, asset);
        
        // Update metadata to remove the dependency
        if (m_Registry) {
            AssetMetadata* metadata = m_Registry->GetAsset(dependent);
            if (metadata) {
                metadata->dependencies.erase(
                    std::remove(metadata->dependencies.begin(), metadata->dependencies.end(), asset),
                    metadata->dependencies.end()
                );
                m_Registry->UpdateAsset(dependent, *metadata);
            }
        }
    }
}

bool AssetDeleter::MoveToTrash(const std::vector<GUID>& assets) {
    if (m_TrashPath.empty()) {
        Log::Error("Trash path not set");
        return false;
    }
    
    // Create trash directory if it doesn't exist
    if (!std::filesystem::exists(m_TrashPath)) {
        std::filesystem::create_directories(m_TrashPath);
    }
    
    bool allSuccess = true;
    for (const auto& asset : assets) {
        if (!MoveAssetToTrash(asset)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool AssetDeleter::MoveAssetToTrash(const GUID& asset) {
    if (!m_Registry || !m_VFS) {
        return false;
    }
    
    const AssetMetadata* metadata = m_Registry->GetAsset(asset);
    if (!metadata) {
        return false;
    }
    
    std::filesystem::path sourcePath = m_VFS->ResolveVirtualPath(metadata->virtualPath);
    
    if (!std::filesystem::exists(sourcePath)) {
        Log::Warn("Asset file does not exist: " + metadata->virtualPath);
        return true; // Consider it successful
    }
    
    try {
        std::filesystem::path destPath = m_TrashPath / sourcePath.filename();
        
        // Handle name conflicts
        int counter = 1;
        while (std::filesystem::exists(destPath)) {
            std::string stem = sourcePath.stem().string();
            std::string ext = sourcePath.extension().string();
            destPath = m_TrashPath / (stem + "_" + std::to_string(counter) + ext);
            counter++;
        }
        
        std::filesystem::rename(sourcePath, destPath);
        
        // Also move .meta file if it exists
        std::filesystem::path metaSource = sourcePath.string() + ".meta";
        if (std::filesystem::exists(metaSource)) {
            std::filesystem::path metaDest = destPath.string() + ".meta";
            std::filesystem::rename(metaSource, metaDest);
        }
        
        Log::Info("Moved asset to trash: " + metadata->virtualPath);
        return true;
    } catch (const std::exception& e) {
        Log::Error("Failed to move asset to trash: " + std::string(e.what()));
        return false;
    }
}

bool AssetDeleter::RestoreFromTrash(const std::vector<GUID>& assets) {
    bool allSuccess = true;
    for (const auto& asset : assets) {
        if (!RestoreAssetFromTrash(asset)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool AssetDeleter::RestoreAssetFromTrash(const GUID& asset) {
    // This would require tracking where assets were moved from
    // For now, this is a placeholder
    Log::Warn("Restore from trash not fully implemented");
    return false;
}

bool AssetDeleter::PermanentlyDelete(const std::vector<GUID>& assets) {
    // Delete assets from trash directory
    // This is similar to DeleteAssets but operates on trash directory
    return DeleteAssets(assets, DeletionMode::SafeOnly).success;
}

} // namespace LGE

