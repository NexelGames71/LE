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
#include <vector>
#include <string>

namespace LGE {

// Forward declarations
class DependencyGraph;
class AssetRegistry;

class AssetDeletionValidator {
public:
    struct DeletionInfo {
        GUID asset;
        bool canSafelyDelete;
        std::vector<GUID> blockingDependents;
        std::string reason;
        
        DeletionInfo() : canSafelyDelete(false) {}
    };

private:
    DependencyGraph* m_Graph;
    AssetRegistry* m_Registry;

public:
    AssetDeletionValidator(DependencyGraph* graph, AssetRegistry* registry);
    
    // Check if assets can be safely deleted
    std::vector<DeletionInfo> ValidateDeletion(const std::vector<GUID>& assets);
    
    // Check single asset
    DeletionInfo ValidateSingleAsset(const GUID& asset);
    
    // Get user confirmation for deletion with warnings
    // This would typically show a dialog, but for now we'll return a boolean
    // The actual UI implementation would be in a separate UI class
    bool CanDeleteSafely(const std::vector<DeletionInfo>& info) const;
};

} // namespace LGE

