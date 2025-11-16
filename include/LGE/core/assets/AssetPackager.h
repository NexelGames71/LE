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
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <ctime>

namespace LGE {

// Forward declarations
class AssetRegistry;
class DependencyGraph;

class AssetPackager {
public:
    struct PackageManifest {
        std::string packageName;
        std::string version;
        std::vector<GUID> includedAssets;
        std::map<GUID, std::string> assetPaths;
        std::time_t buildTime;
        
        PackageManifest()
            : version("1.0.0")
            , buildTime(0)
        {}
        
        std::string ToJson() const;
        static PackageManifest FromJson(const std::string& json);
    };
    
    struct PackageOptions {
        bool includeSourceAssets = false;
        bool compressAssets = true;
        bool stripEditorData = true;
        bool includeDependencies = true;
        std::string compressionFormat = "none"; // "none", "zlib", "zstd"
        int compressionLevel = 5;
        
        PackageOptions() {}
    };

private:
    AssetRegistry* m_Registry;
    DependencyGraph* m_Graph;

public:
    AssetPackager(AssetRegistry* reg, DependencyGraph* graph);
    
    // Create package from asset list
    bool CreatePackage(
        const std::filesystem::path& outputPath,
        const std::vector<GUID>& assets,
        const PackageOptions& options
    );
    
    // Create package from scene (includes all dependencies)
    bool PackageScene(
        const std::filesystem::path& outputPath,
        const GUID& sceneAsset,
        const PackageOptions& options
    );
    
    // Extract package
    bool ExtractPackage(
        const std::filesystem::path& packagePath,
        const std::filesystem::path& destinationPath
    );
    
    // Get package info without extracting
    PackageManifest ReadPackageManifest(const std::filesystem::path& packagePath);

private:
    // Helper methods
    std::vector<GUID> CollectDependencies(const std::vector<GUID>& assets);
    bool CopyAssetToPackage(const GUID& asset, const std::filesystem::path& packageDir, const PackageOptions& options);
    bool WriteManifest(const PackageManifest& manifest, const std::filesystem::path& manifestPath);
    PackageManifest ReadManifestFromFile(const std::filesystem::path& manifestPath);
};

} // namespace LGE

