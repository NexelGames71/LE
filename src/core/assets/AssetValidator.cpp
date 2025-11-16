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

#include "LGE/core/assets/AssetValidator.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/assets/DependencyGraph.h"
#include "LGE/core/filesystem/VirtualFileSystem.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

namespace LGE {

AssetValidator::AssetValidator(AssetRegistry* registry, DependencyGraph* graph)
    : m_Registry(registry)
    , m_Graph(graph)
{
}

std::vector<AssetValidator::ValidationResult> AssetValidator::ValidateAsset(const GUID& guid) {
    std::vector<ValidationResult> results;
    
    if (!m_Registry) {
        return results;
    }
    
    const AssetMetadata* metadata = m_Registry->GetAsset(guid);
    if (!metadata) {
        ValidationResult result;
        result.severity = ValidationSeverity::Error;
        result.asset = guid;
        result.message = "Asset not found in registry";
        results.push_back(result);
        return results;
    }
    
    // Check if file exists
    // This would require VFS, but for now we'll skip
    
    // Check for invalid GUID
    if (!metadata->guid.IsValid()) {
        ValidationResult result;
        result.severity = ValidationSeverity::Error;
        result.asset = guid;
        result.message = "Invalid GUID";
        results.push_back(result);
    }
    
    // Check for empty name
    if (metadata->name.empty()) {
        ValidationResult result;
        result.severity = ValidationSeverity::Warning;
        result.asset = guid;
        result.message = "Asset has empty name";
        results.push_back(result);
    }
    
    // Check dependencies
    if (m_Graph) {
        std::vector<GUID> deps = m_Graph->GetDependencies(guid);
        for (const auto& dep : deps) {
            const AssetMetadata* depMetadata = m_Registry->GetAsset(dep);
            if (!depMetadata) {
                ValidationResult result;
                result.severity = ValidationSeverity::Error;
                result.asset = guid;
                result.message = "Missing dependency: " + dep.ToString();
                result.details = "Asset depends on " + dep.ToString() + " which is not in registry";
                results.push_back(result);
            }
        }
    }
    
    return results;
}

std::vector<AssetValidator::ValidationResult> AssetValidator::ValidateProject(
    std::function<void(int, int)> progressCallback)
{
    std::vector<ValidationResult> allResults;
    
    if (!m_Registry) {
        return allResults;
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
            if (metadata) {
                std::vector<ValidationResult> assetResults = ValidateAsset(metadata->guid);
                allResults.insert(allResults.end(), assetResults.begin(), assetResults.end());
                processed++;
                
                if (progressCallback) {
                    progressCallback(processed, totalAssets);
                }
            }
        }
    }
    
    // Run common checks
    std::vector<ValidationResult> missingDeps = CheckMissingDependencies();
    allResults.insert(allResults.end(), missingDeps.begin(), missingDeps.end());
    
    std::vector<ValidationResult> circularDeps = CheckCircularDependencies();
    allResults.insert(allResults.end(), circularDeps.begin(), circularDeps.end());
    
    std::vector<ValidationResult> duplicateNames = CheckDuplicateNames();
    allResults.insert(allResults.end(), duplicateNames.begin(), duplicateNames.end());
    
    return allResults;
}

std::vector<AssetValidator::ValidationResult> AssetValidator::CheckMissingDependencies() {
    std::vector<ValidationResult> results;
    
    if (!m_Registry || !m_Graph) {
        return results;
    }
    
    std::vector<AssetType> allTypes = {
        AssetType::Texture, AssetType::Model, AssetType::Material,
        AssetType::Shader, AssetType::Script, AssetType::Audio,
        AssetType::Scene, AssetType::Prefab, AssetType::Animation, AssetType::Font
    };
    
    for (AssetType type : allTypes) {
        std::vector<AssetMetadata*> assets = m_Registry->GetAssetsByType(type);
        for (AssetMetadata* metadata : assets) {
            if (!metadata) continue;
            
            for (const auto& depGuid : metadata->dependencies) {
                const AssetMetadata* depMetadata = m_Registry->GetAsset(depGuid);
                if (!depMetadata) {
                    ValidationResult result;
                    result.severity = ValidationSeverity::Error;
                    result.asset = metadata->guid;
                    result.message = "Missing dependency";
                    result.details = "Asset depends on " + depGuid.ToString() + " which is not in registry";
                    result.canAutoFix = false;
                    results.push_back(result);
                }
            }
        }
    }
    
    return results;
}

std::vector<AssetValidator::ValidationResult> AssetValidator::CheckCircularDependencies() {
    std::vector<ValidationResult> results;
    
    if (!m_Registry || !m_Graph) {
        return results;
    }
    
    std::vector<AssetType> allTypes = {
        AssetType::Texture, AssetType::Model, AssetType::Material,
        AssetType::Shader, AssetType::Script, AssetType::Audio,
        AssetType::Scene, AssetType::Prefab, AssetType::Animation, AssetType::Font
    };
    
    for (AssetType type : allTypes) {
        std::vector<AssetMetadata*> assets = m_Registry->GetAssetsByType(type);
        for (AssetMetadata* metadata : assets) {
            if (!metadata) continue;
            
            if (m_Graph->HasCircularDependency(metadata->guid)) {
                ValidationResult result;
                result.severity = ValidationSeverity::Warning;
                result.asset = metadata->guid;
                result.message = "Circular dependency detected";
                result.details = "Asset has circular dependency in dependency graph";
                result.canAutoFix = false;
                results.push_back(result);
            }
        }
    }
    
    return results;
}

std::vector<AssetValidator::ValidationResult> AssetValidator::CheckDuplicateNames() {
    std::vector<ValidationResult> results;
    
    if (!m_Registry) {
        return results;
    }
    
    std::unordered_map<std::string, std::vector<GUID>> nameMap;
    
    std::vector<AssetType> allTypes = {
        AssetType::Texture, AssetType::Model, AssetType::Material,
        AssetType::Shader, AssetType::Script, AssetType::Audio,
        AssetType::Scene, AssetType::Prefab, AssetType::Animation, AssetType::Font
    };
    
    for (AssetType type : allTypes) {
        std::vector<AssetMetadata*> assets = m_Registry->GetAssetsByType(type);
        for (AssetMetadata* metadata : assets) {
            if (!metadata) continue;
            
            nameMap[metadata->name].push_back(metadata->guid);
        }
    }
    
    for (const auto& pair : nameMap) {
        if (pair.second.size() > 1) {
            for (const auto& guid : pair.second) {
                ValidationResult result;
                result.severity = ValidationSeverity::Warning;
                result.asset = guid;
                result.message = "Duplicate asset name";
                result.details = "Multiple assets share the name: " + pair.first;
                result.canAutoFix = false;
                results.push_back(result);
            }
        }
    }
    
    return results;
}

std::vector<AssetValidator::ValidationResult> AssetValidator::CheckInvalidMetadata() {
    std::vector<ValidationResult> results;
    
    if (!m_Registry) {
        return results;
    }
    
    std::vector<AssetType> allTypes = {
        AssetType::Texture, AssetType::Model, AssetType::Material,
        AssetType::Shader, AssetType::Script, AssetType::Audio,
        AssetType::Scene, AssetType::Prefab, AssetType::Animation, AssetType::Font
    };
    
    for (AssetType type : allTypes) {
        std::vector<AssetMetadata*> assets = m_Registry->GetAssetsByType(type);
        for (AssetMetadata* metadata : assets) {
            if (!metadata) continue;
            
            if (!metadata->guid.IsValid()) {
                ValidationResult result;
                result.severity = ValidationSeverity::Error;
                result.asset = GUID::Invalid();
                result.message = "Invalid GUID in metadata";
                result.details = "Asset has invalid GUID";
                results.push_back(result);
            }
            
            if (metadata->virtualPath.empty()) {
                ValidationResult result;
                result.severity = ValidationSeverity::Error;
                result.asset = metadata->guid;
                result.message = "Empty virtual path";
                result.details = "Asset has no virtual path";
                results.push_back(result);
            }
        }
    }
    
    return results;
}

std::vector<AssetValidator::ValidationResult> AssetValidator::CheckOrphanedMetaFiles() {
    // This would check for .meta files without corresponding assets
    // Implementation would require VFS and file system scanning
    return {};
}

bool AssetValidator::AutoFixIssues(const std::vector<ValidationResult>& results) {
    bool allFixed = true;
    
    for (const auto& result : results) {
        if (result.canAutoFix && result.autoFixFunc) {
            if (!result.autoFixFunc()) {
                allFixed = false;
            }
        }
    }
    
    return allFixed;
}

AssetValidator::ValidationSummary AssetValidator::GetSummary(const std::vector<ValidationResult>& results) const {
    ValidationSummary summary;
    
    for (const auto& result : results) {
        switch (result.severity) {
            case ValidationSeverity::Info:
                summary.infoCount++;
                break;
            case ValidationSeverity::Warning:
                summary.warningCount++;
                break;
            case ValidationSeverity::Error:
                summary.errorCount++;
                break;
        }
        
        if (result.canAutoFix) {
            summary.autoFixableCount++;
        }
    }
    
    return summary;
}

} // namespace LGE

