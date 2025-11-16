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

#include "LGE/core/assets/DependencyScanner.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/assets/DependencyGraph.h"
#include "LGE/core/filesystem/VirtualFileSystem.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <algorithm>

namespace LGE {

DependencyScanner::DependencyScanner(AssetRegistry* reg, DependencyGraph* graph)
    : m_Registry(reg)
    , m_Graph(graph)
{
}

std::vector<GUID> DependencyScanner::ScanAsset(const GUID& asset) {
    if (!m_Registry) {
        Log::Error("AssetRegistry is null in DependencyScanner");
        return {};
    }
    
    const AssetMetadata* metadata = m_Registry->GetAsset(asset);
    if (!metadata) {
        Log::Warn("Asset not found for dependency scanning: " + asset.ToString());
        return {};
    }
    
    return ScanByType(asset, metadata->type);
}

std::vector<GUID> DependencyScanner::ScanByType(const GUID& asset, AssetType type) {
    switch (type) {
        case AssetType::Material:
            return ScanMaterialDependencies(asset);
        case AssetType::Scene:
            return ScanSceneDependencies(asset);
        case AssetType::Prefab:
            return ScanPrefabDependencies(asset);
        default:
            // For other types, check if metadata already has dependencies
            const AssetMetadata* metadata = m_Registry->GetAsset(asset);
            if (metadata) {
                return metadata->dependencies;
            }
            return {};
    }
}

std::vector<GUID> DependencyScanner::ScanMaterialDependencies(const GUID& materialGuid) {
    std::vector<GUID> dependencies;
    
    const AssetMetadata* metadata = m_Registry->GetAsset(materialGuid);
    if (!metadata) {
        return dependencies;
    }
    
    // Materials typically depend on textures and shaders
    // Check importSettings for texture references (it's stored as a JSON string)
    if (!metadata->importSettings.empty()) {
        // Look for common texture property names
        std::vector<std::string> textureKeys = {"diffuseTexture", "normalTexture", "specularTexture", 
                                                 "albedoTexture", "metallicTexture", "roughnessTexture",
                                                 "emissiveTexture", "occlusionTexture"};
        
        for (const auto& key : textureKeys) {
            std::string searchKey = "\"" + key + "\"";
            size_t keyPos = metadata->importSettings.find(searchKey);
            if (keyPos != std::string::npos) {
                // Find the value after the colon
                size_t colonPos = metadata->importSettings.find(':', keyPos);
                if (colonPos != std::string::npos) {
                    // Find the quoted value
                    size_t valueStart = metadata->importSettings.find('"', colonPos);
                    if (valueStart != std::string::npos) {
                        size_t valueEnd = metadata->importSettings.find('"', valueStart + 1);
                        if (valueEnd != std::string::npos) {
                            std::string textureGuidStr = metadata->importSettings.substr(valueStart + 1, valueEnd - valueStart - 1);
                            GUID textureGuid = GUID::FromString(textureGuidStr);
                            if (textureGuid.IsValid()) {
                                dependencies.push_back(textureGuid);
                            }
                        }
                    }
                }
            }
        }
        
        // Check for shader reference
        size_t shaderPos = metadata->importSettings.find("\"shader\"");
        if (shaderPos != std::string::npos) {
            size_t colonPos = metadata->importSettings.find(':', shaderPos);
            if (colonPos != std::string::npos) {
                size_t valueStart = metadata->importSettings.find('"', colonPos);
                if (valueStart != std::string::npos) {
                    size_t valueEnd = metadata->importSettings.find('"', valueStart + 1);
                    if (valueEnd != std::string::npos) {
                        std::string shaderGuidStr = metadata->importSettings.substr(valueStart + 1, valueEnd - valueStart - 1);
                        GUID shaderGuid = GUID::FromString(shaderGuidStr);
                        if (shaderGuid.IsValid()) {
                            dependencies.push_back(shaderGuid);
                        }
                    }
                }
            }
        }
    }
    
    // Also check metadata dependencies
    dependencies.insert(dependencies.end(), metadata->dependencies.begin(), metadata->dependencies.end());
    
    // Remove duplicates
    std::sort(dependencies.begin(), dependencies.end());
    dependencies.erase(std::unique(dependencies.begin(), dependencies.end()), dependencies.end());
    
    return dependencies;
}

std::vector<GUID> DependencyScanner::ScanSceneDependencies(const GUID& sceneGuid) {
    std::vector<GUID> dependencies;
    
    const AssetMetadata* metadata = m_Registry->GetAsset(sceneGuid);
    if (!metadata) {
        return dependencies;
    }
    
    // Scenes can reference many assets (models, materials, textures, prefabs, etc.)
    // For now, we'll check the metadata dependencies and try to parse the scene file
    dependencies = metadata->dependencies;
    
    // TODO: Parse scene file format to extract all referenced assets
    // This would require reading the actual scene file and parsing it
    
    // Remove duplicates
    std::sort(dependencies.begin(), dependencies.end());
    dependencies.erase(std::unique(dependencies.begin(), dependencies.end()), dependencies.end());
    
    return dependencies;
}

std::vector<GUID> DependencyScanner::ScanPrefabDependencies(const GUID& prefabGuid) {
    std::vector<GUID> dependencies;
    
    const AssetMetadata* metadata = m_Registry->GetAsset(prefabGuid);
    if (!metadata) {
        return dependencies;
    }
    
    // Prefabs can reference models, materials, textures, other prefabs, etc.
    dependencies = metadata->dependencies;
    
    // TODO: Parse prefab file format to extract all referenced assets
    
    // Remove duplicates
    std::sort(dependencies.begin(), dependencies.end());
    dependencies.erase(std::unique(dependencies.begin(), dependencies.end()), dependencies.end());
    
    return dependencies;
}

std::vector<GUID> DependencyScanner::ExtractGUIDsFromJson(const std::string& jsonString, const std::string& key) {
    std::vector<GUID> result;
    
    // Find the key in the JSON string
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = jsonString.find(searchKey);
    if (keyPos == std::string::npos) {
        return result;
    }
    
    // Find array start
    size_t arrayStart = jsonString.find('[', keyPos);
    if (arrayStart == std::string::npos) {
        return result;
    }
    
    size_t arrayEnd = jsonString.find(']', arrayStart);
    if (arrayEnd == std::string::npos) {
        return result;
    }
    
    // Parse GUIDs in array
    std::string arrayContent = jsonString.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
    size_t pos = 0;
    while ((pos = arrayContent.find('"', pos)) != std::string::npos) {
        size_t guidStart = pos;
        size_t guidEnd = arrayContent.find('"', guidStart + 1);
        if (guidEnd == std::string::npos) {
            break;
        }
        
        std::string guidStr = arrayContent.substr(guidStart + 1, guidEnd - guidStart - 1);
        GUID guid = GUID::FromString(guidStr);
        if (guid.IsValid()) {
            result.push_back(guid);
        }
        
        pos = guidEnd + 1;
    }
    
    return result;
}

void DependencyScanner::ScanAll(std::function<void(int, int)> progressCallback) {
    if (!m_Registry || !m_Graph) {
        Log::Error("AssetRegistry or DependencyGraph is null in DependencyScanner");
        return;
    }
    
    // Get all assets
    std::vector<AssetType> allTypes = {
        AssetType::Texture, AssetType::Model, AssetType::Material,
        AssetType::Shader, AssetType::Script, AssetType::Audio,
        AssetType::Scene, AssetType::Prefab, AssetType::Animation, AssetType::Font
    };
    
    int totalAssets = 0;
    for (AssetType type : allTypes) {
        totalAssets += static_cast<int>(m_Registry->GetAssetCountByType(type));
    }
    
    int processed = 0;
    
    for (AssetType type : allTypes) {
        std::vector<AssetMetadata*> assets = m_Registry->GetAssetsByType(type);
        for (AssetMetadata* metadata : assets) {
            if (metadata && metadata->guid.IsValid()) {
                RescanAsset(metadata->guid);
                processed++;
                
                if (progressCallback) {
                    progressCallback(processed, totalAssets);
                }
            }
        }
    }
    
    Log::Info("Dependency scanning complete. Scanned " + std::to_string(processed) + " assets.");
}

void DependencyScanner::RescanAsset(const GUID& asset) {
    if (!m_Graph) {
        return;
    }
    
    // Remove old dependencies
    m_Graph->RemoveDependencies(asset);
    
    // Scan for new dependencies
    std::vector<GUID> dependencies = ScanAsset(asset);
    
    // Add new dependencies to graph
    for (const auto& dep : dependencies) {
        m_Graph->AddDependency(asset, dep);
    }
    
    // Update metadata dependencies
    if (m_Registry) {
        AssetMetadata* metadata = m_Registry->GetAsset(asset);
        if (metadata) {
            metadata->dependencies = dependencies;
            m_Registry->UpdateAsset(asset, *metadata);
            m_Registry->SetDirty(true);
            m_Registry->SaveToCache();
        }
    }
    
    Log::Info("Rescanned dependencies for asset: " + asset.ToString() + " (" + std::to_string(dependencies.size()) + " dependencies)");
}

} // namespace LGE

