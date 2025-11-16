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
#include "LGE/core/assets/AssetDeletionValidator.h"
#include <vector>
#include <string>
#include <filesystem>

namespace LGE {

// Forward declarations
class AssetRegistry;
class DependencyGraph;
class VirtualFileSystem;

class AssetDeleter {
public:
    enum class DeletionMode {
        SafeOnly,           // Only delete if no references
        BreakReferences,    // Delete and break references
        CascadeDelete       // Delete and all dependents
    };
    
    struct DeletionResult {
        bool success;
        std::vector<GUID> deletedAssets;
        std::vector<GUID> failedAssets;
        std::string errorMessage;
        
        DeletionResult() : success(false) {}
    };

private:
    AssetRegistry* m_Registry;
    DependencyGraph* m_Graph;
    VirtualFileSystem* m_VFS;
    AssetDeletionValidator m_Validator;
    
    std::filesystem::path m_TrashPath;

public:
    AssetDeleter(AssetRegistry* registry, DependencyGraph* graph, VirtualFileSystem* vfs);
    
    // Delete assets with specified mode
    DeletionResult DeleteAssets(
        const std::vector<GUID>& assets, 
        DeletionMode mode = DeletionMode::SafeOnly
    );
    
    // Move to trash instead of permanent deletion
    bool MoveToTrash(const std::vector<GUID>& assets);
    
    // Restore from trash
    bool RestoreFromTrash(const std::vector<GUID>& assets);
    
    // Set trash directory path
    void SetTrashPath(const std::filesystem::path& path) { m_TrashPath = path; }
    const std::filesystem::path& GetTrashPath() const { return m_TrashPath; }
    
    // Permanently delete from trash
    bool PermanentlyDelete(const std::vector<GUID>& assets);

private:
    // Helper methods
    bool DeleteAssetFile(const GUID& asset);
    bool MoveAssetToTrash(const GUID& asset);
    bool RestoreAssetFromTrash(const GUID& asset);
    void BreakReferences(const GUID& asset);
};

} // namespace LGE

