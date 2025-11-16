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

#include "LGE/core/assets/AssetDeletionValidator.h"
#include "LGE/core/assets/DependencyGraph.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/Log.h"

namespace LGE {

AssetDeletionValidator::AssetDeletionValidator(DependencyGraph* graph, AssetRegistry* registry)
    : m_Graph(graph)
    , m_Registry(registry)
{
}

AssetDeletionValidator::DeletionInfo AssetDeletionValidator::ValidateSingleAsset(const GUID& asset) {
    DeletionInfo info;
    info.asset = asset;
    
    if (!m_Graph) {
        info.canSafelyDelete = true;
        info.reason = "No dependency graph available";
        return info;
    }
    
    // Check if any assets depend on this one
    std::vector<GUID> dependents = m_Graph->GetDependents(asset);
    
    if (dependents.empty()) {
        info.canSafelyDelete = true;
        info.reason = "No assets depend on this asset";
    } else {
        info.canSafelyDelete = false;
        info.blockingDependents = dependents;
        info.reason = "Asset is referenced by " + std::to_string(dependents.size()) + " other asset(s)";
    }
    
    return info;
}

std::vector<AssetDeletionValidator::DeletionInfo> AssetDeletionValidator::ValidateDeletion(const std::vector<GUID>& assets) {
    std::vector<DeletionInfo> results;
    
    for (const auto& asset : assets) {
        results.push_back(ValidateSingleAsset(asset));
    }
    
    return results;
}

bool AssetDeletionValidator::CanDeleteSafely(const std::vector<DeletionInfo>& info) const {
    for (const auto& item : info) {
        if (!item.canSafelyDelete) {
            return false;
        }
    }
    return true;
}

} // namespace LGE

