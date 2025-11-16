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
#include <map>
#include <string>

namespace LGE {

// Forward declarations
class AssetRegistry;
class DependencyGraph;

struct AssetReference {
    GUID referencingAsset;
    AssetType assetType;
    std::string assetName;
    std::string assetPath;
    
    enum class ReferenceType {
        Direct,      // Direct property reference
        Indirect,    // Referenced through another asset
        Scene,       // Used in a scene
        Code         // Referenced in script/code
    };
    
    ReferenceType referenceType;
    std::string propertyPath; // Where in the asset it's referenced
    
    AssetReference()
        : referenceType(ReferenceType::Direct)
        , assetType(AssetType::Unknown)
    {}
};

class AssetReferenceFinder {
private:
    AssetRegistry* m_Registry;
    DependencyGraph* m_Graph;

public:
    AssetReferenceFinder(AssetRegistry* registry, DependencyGraph* graph);
    
    // Find all references to an asset
    std::vector<AssetReference> FindReferences(const GUID& asset);
    
    // Find references in a specific asset
    std::vector<AssetReference> FindReferencesIn(const GUID& searchIn, const GUID& target);
    
    // Find unused assets (nothing references them)
    std::vector<GUID> FindUnusedAssets();
    
    // Find missing references (broken links)
    std::map<GUID, std::vector<GUID>> FindMissingReferences();
    
    // Get reference count for an asset
    size_t GetReferenceCount(const GUID& asset) const;
};

} // namespace LGE

