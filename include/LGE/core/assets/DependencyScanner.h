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
#include "LGE/core/assets/AssetMetadata.h"
#include <vector>
#include <functional>

namespace LGE {

// Forward declarations
class AssetRegistry;
class DependencyGraph;

class DependencyScanner {
private:
    AssetRegistry* m_Registry;
    DependencyGraph* m_Graph;
    
    // Type-specific scanners
    std::vector<GUID> ScanMaterialDependencies(const GUID& materialGuid);
    std::vector<GUID> ScanSceneDependencies(const GUID& sceneGuid);
    std::vector<GUID> ScanPrefabDependencies(const GUID& prefabGuid);
    
    // Helper to extract GUIDs from JSON (for asset files that reference other assets)
    std::vector<GUID> ExtractGUIDsFromJson(const std::string& jsonString, const std::string& key = "dependencies");

public:
    DependencyScanner(AssetRegistry* reg, DependencyGraph* graph);
    
    // Scan single asset for dependencies
    std::vector<GUID> ScanAsset(const GUID& asset);
    
    // Scan all assets
    void ScanAll(std::function<void(int, int)> progressCallback = nullptr);
    
    // Rescan specific asset (remove old deps and scan again)
    void RescanAsset(const GUID& asset);
    
    // Get scanner for specific asset type
    std::vector<GUID> ScanByType(const GUID& asset, AssetType type);
};

} // namespace LGE

