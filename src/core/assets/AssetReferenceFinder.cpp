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

#include "LGE/core/assets/AssetReferenceFinder.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/assets/DependencyGraph.h"
#include "LGE/core/Log.h"
#include <algorithm>

namespace LGE {

AssetReferenceFinder::AssetReferenceFinder(AssetRegistry* registry, DependencyGraph* graph)
    : m_Registry(registry)
    , m_Graph(graph)
{
}

std::vector<AssetReference> AssetReferenceFinder::FindReferences(const GUID& asset) {
    std::vector<AssetReference> references;
    
    if (!m_Graph || !m_Registry) {
        return references;
    }
    
    // Get all assets that depend on this one (these are the references)
    std::vector<GUID> dependents = m_Graph->GetDependents(asset);
    
    for (const auto& dependentGuid : dependents) {
        const AssetMetadata* metadata = m_Registry->GetAsset(dependentGuid);
        if (!metadata) {
            continue;
        }
        
        AssetReference ref;
        ref.referencingAsset = dependentGuid;
        ref.assetType = metadata->type;
        ref.assetName = metadata->name;
        ref.assetPath = metadata->virtualPath;
        ref.referenceType = AssetReference::ReferenceType::Direct;
        
        // Determine reference type based on asset type
        if (metadata->type == AssetType::Scene) {
            ref.referenceType = AssetReference::ReferenceType::Scene;
        } else if (metadata->type == AssetType::Script) {
            ref.referenceType = AssetReference::ReferenceType::Code;
        }
        
        references.push_back(ref);
    }
    
    return references;
}

std::vector<AssetReference> AssetReferenceFinder::FindReferencesIn(const GUID& searchIn, const GUID& target) {
    std::vector<AssetReference> references;
    
    if (!m_Registry) {
        return references;
    }
    
    const AssetMetadata* searchMetadata = m_Registry->GetAsset(searchIn);
    if (!searchMetadata) {
        return references;
    }
    
    // Check if target is in the dependencies
    auto it = std::find(searchMetadata->dependencies.begin(), searchMetadata->dependencies.end(), target);
    if (it != searchMetadata->dependencies.end()) {
        AssetReference ref;
        ref.referencingAsset = searchIn;
        ref.assetType = searchMetadata->type;
        ref.assetName = searchMetadata->name;
        ref.assetPath = searchMetadata->virtualPath;
        ref.referenceType = AssetReference::ReferenceType::Direct;
        references.push_back(ref);
    }
    
    return references;
}

std::vector<GUID> AssetReferenceFinder::FindUnusedAssets() {
    std::vector<GUID> unused;
    
    if (!m_Registry || !m_Graph) {
        return unused;
    }
    
    // Get all assets
    std::vector<AssetType> allTypes = {
        AssetType::Texture, AssetType::Model, AssetType::Material,
        AssetType::Shader, AssetType::Script, AssetType::Audio,
        AssetType::Scene, AssetType::Prefab, AssetType::Animation, AssetType::Font
    };
    
    for (AssetType type : allTypes) {
        std::vector<AssetMetadata*> assets = m_Registry->GetAssetsByType(type);
        for (AssetMetadata* metadata : assets) {
            if (!metadata) {
                continue;
            }
            
            // Check if any assets depend on this one
            std::vector<GUID> dependents = m_Graph->GetDependents(metadata->guid);
            if (dependents.empty()) {
                unused.push_back(metadata->guid);
            }
        }
    }
    
    return unused;
}

std::map<GUID, std::vector<GUID>> AssetReferenceFinder::FindMissingReferences() {
    std::map<GUID, std::vector<GUID>> missingRefs;
    
    if (!m_Registry || !m_Graph) {
        return missingRefs;
    }
    
    // Get all assets
    std::vector<AssetType> allTypes = {
        AssetType::Texture, AssetType::Model, AssetType::Material,
        AssetType::Shader, AssetType::Script, AssetType::Audio,
        AssetType::Scene, AssetType::Prefab, AssetType::Animation, AssetType::Font
    };
    
    for (AssetType type : allTypes) {
        std::vector<AssetMetadata*> assets = m_Registry->GetAssetsByType(type);
        for (AssetMetadata* metadata : assets) {
            if (!metadata) {
                continue;
            }
            
            // Check each dependency
            for (const auto& depGuid : metadata->dependencies) {
                const AssetMetadata* depMetadata = m_Registry->GetAsset(depGuid);
                if (!depMetadata) {
                    // Missing reference
                    missingRefs[metadata->guid].push_back(depGuid);
                }
            }
        }
    }
    
    return missingRefs;
}

size_t AssetReferenceFinder::GetReferenceCount(const GUID& asset) const {
    if (!m_Graph) {
        return 0;
    }
    
    return m_Graph->GetDependents(asset).size();
}

} // namespace LGE

