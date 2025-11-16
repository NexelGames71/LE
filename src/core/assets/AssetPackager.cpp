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

#include "LGE/core/assets/AssetPackager.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/assets/DependencyGraph.h"
#include "LGE/core/filesystem/VirtualFileSystem.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <chrono>
#include <unordered_set>

namespace LGE {

AssetPackager::AssetPackager(AssetRegistry* reg, DependencyGraph* graph)
    : m_Registry(reg)
    , m_Graph(graph)
{
}

std::vector<GUID> AssetPackager::CollectDependencies(const std::vector<GUID>& assets) {
    std::vector<GUID> allAssets = assets;
    std::unordered_set<GUID> addedAssets(assets.begin(), assets.end());
    
    if (!m_Graph) {
        return allAssets;
    }
    
    // Collect all dependencies recursively
    for (const auto& asset : assets) {
        std::vector<GUID> deps = m_Graph->GetAllDependencies(asset, true);
        for (const auto& dep : deps) {
            if (addedAssets.insert(dep).second) {
                allAssets.push_back(dep);
            }
        }
    }
    
    return allAssets;
}

bool AssetPackager::CreatePackage(
    const std::filesystem::path& outputPath,
    const std::vector<GUID>& assets,
    const PackageOptions& options)
{
    if (!m_Registry) {
        Log::Error("AssetRegistry is null in AssetPackager");
        return false;
    }
    
    // Collect all assets to package (including dependencies if requested)
    std::vector<GUID> assetsToPackage = assets;
    if (options.includeDependencies) {
        assetsToPackage = CollectDependencies(assets);
    }
    
    // Create package directory
    if (!std::filesystem::exists(outputPath.parent_path())) {
        std::filesystem::create_directories(outputPath.parent_path());
    }
    
    std::filesystem::path packageDir = outputPath;
    packageDir.replace_extension(""); // Remove extension to create directory
    
    if (std::filesystem::exists(packageDir)) {
        std::filesystem::remove_all(packageDir);
    }
    std::filesystem::create_directories(packageDir);
    
    // Create assets subdirectory
    std::filesystem::path assetsDir = packageDir / "assets";
    std::filesystem::create_directories(assetsDir);
    
    // Copy assets
    PackageManifest manifest;
    manifest.packageName = outputPath.stem().string();
    manifest.version = "1.0.0";
    manifest.buildTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    
    for (const auto& assetGuid : assetsToPackage) {
        if (CopyAssetToPackage(assetGuid, assetsDir, options)) {
            manifest.includedAssets.push_back(assetGuid);
            
            const AssetMetadata* metadata = m_Registry->GetAsset(assetGuid);
            if (metadata) {
                manifest.assetPaths[assetGuid] = metadata->virtualPath;
            }
        }
    }
    
    // Write manifest
    std::filesystem::path manifestPath = packageDir / "manifest.json";
    if (!WriteManifest(manifest, manifestPath)) {
        Log::Error("Failed to write package manifest");
        return false;
    }
    
    Log::Info("Package created: " + outputPath.string() + " with " + std::to_string(manifest.includedAssets.size()) + " assets");
    return true;
}

bool AssetPackager::PackageScene(
    const std::filesystem::path& outputPath,
    const GUID& sceneAsset,
    const PackageOptions& options)
{
    std::vector<GUID> sceneAssets = { sceneAsset };
    return CreatePackage(outputPath, sceneAssets, options);
}

bool AssetPackager::CopyAssetToPackage(const GUID& asset, const std::filesystem::path& packageDir, const PackageOptions& options) {
    if (!m_Registry) {
        return false;
    }
    
    const AssetMetadata* metadata = m_Registry->GetAsset(asset);
    if (!metadata) {
        Log::Warn("Asset not found for packaging: " + asset.ToString());
        return false;
    }
    
    // Get physical path (would need VFS, but for now assume direct path)
    std::filesystem::path sourcePath = metadata->virtualPath;
    
    // In a real implementation, we'd resolve through VFS
    // For now, assume virtualPath is relative to project root
    
    if (!std::filesystem::exists(sourcePath)) {
        Log::Warn("Asset file not found: " + metadata->virtualPath);
        return false;
    }
    
    // Create destination path maintaining directory structure
    std::filesystem::path destPath = packageDir / metadata->virtualPath;
    std::filesystem::create_directories(destPath.parent_path());
    
    try {
        std::filesystem::copy_file(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing);
        
        // Copy .meta file if it exists
        std::filesystem::path metaSource = sourcePath.string() + ".meta";
        if (std::filesystem::exists(metaSource)) {
            std::filesystem::path metaDest = destPath.string() + ".meta";
            std::filesystem::copy_file(metaSource, metaDest, std::filesystem::copy_options::overwrite_existing);
        }
        
        return true;
    } catch (const std::exception& e) {
        Log::Error("Failed to copy asset: " + std::string(e.what()));
        return false;
    }
}

bool AssetPackager::WriteManifest(const PackageManifest& manifest, const std::filesystem::path& manifestPath) {
    std::ostringstream json;
    json << "{\n";
    json << "  \"packageName\": \"" << manifest.packageName << "\",\n";
    json << "  \"version\": \"" << manifest.version << "\",\n";
    json << "  \"buildTime\": " << manifest.buildTime << ",\n";
    json << "  \"includedAssets\": [\n";
    for (size_t i = 0; i < manifest.includedAssets.size(); ++i) {
        json << "    \"" << manifest.includedAssets[i].ToString() << "\"";
        if (i < manifest.includedAssets.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ],\n";
    json << "  \"assetPaths\": {\n";
    size_t pathIndex = 0;
    for (const auto& pair : manifest.assetPaths) {
        json << "    \"" << pair.first.ToString() << "\": \"" << pair.second << "\"";
        if (pathIndex < manifest.assetPaths.size() - 1) {
            json << ",";
        }
        json << "\n";
        pathIndex++;
    }
    json << "  }\n";
    json << "}\n";
    
    return FileSystem::WriteFile(manifestPath.string(), json.str());
}

AssetPackager::PackageManifest AssetPackager::ReadManifestFromFile(const std::filesystem::path& manifestPath) {
    PackageManifest manifest;
    
    if (!FileSystem::Exists(manifestPath.string())) {
        return manifest;
    }
    
    std::string jsonString = FileSystem::ReadFile(manifestPath.string());
    if (jsonString.empty()) {
        return manifest;
    }
    
    // Parse packageName
    size_t namePos = jsonString.find("\"packageName\"");
    if (namePos != std::string::npos) {
        size_t colonPos = jsonString.find(':', namePos);
        size_t quoteStart = jsonString.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = jsonString.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                manifest.packageName = jsonString.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Parse version
    size_t versionPos = jsonString.find("\"version\"");
    if (versionPos != std::string::npos) {
        size_t colonPos = jsonString.find(':', versionPos);
        size_t quoteStart = jsonString.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = jsonString.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                manifest.version = jsonString.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Parse buildTime
    size_t timePos = jsonString.find("\"buildTime\"");
    if (timePos != std::string::npos) {
        size_t colonPos = jsonString.find(':', timePos);
        size_t numStart = jsonString.find_first_of("0123456789", colonPos);
        if (numStart != std::string::npos) {
            size_t numEnd = numStart;
            while (numEnd < jsonString.length() && std::isdigit(jsonString[numEnd])) {
                numEnd++;
            }
            manifest.buildTime = std::stoll(jsonString.substr(numStart, numEnd - numStart));
        }
    }
    
    // Parse includedAssets array
    size_t assetsPos = jsonString.find("\"includedAssets\"");
    if (assetsPos != std::string::npos) {
        size_t arrayStart = jsonString.find('[', assetsPos);
        size_t arrayEnd = jsonString.find(']', arrayStart);
        if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
            std::string arrayContent = jsonString.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            size_t pos = 0;
            while ((pos = arrayContent.find('"', pos)) != std::string::npos) {
                size_t guidStart = pos;
                size_t guidEnd = arrayContent.find('"', guidStart + 1);
                if (guidEnd != std::string::npos) {
                    std::string guidStr = arrayContent.substr(guidStart + 1, guidEnd - guidStart - 1);
                    GUID guid = GUID::FromString(guidStr);
                    if (guid.IsValid()) {
                        manifest.includedAssets.push_back(guid);
                    }
                    pos = guidEnd + 1;
                } else {
                    break;
                }
            }
        }
    }
    
    return manifest;
}

AssetPackager::PackageManifest AssetPackager::ReadPackageManifest(const std::filesystem::path& packagePath) {
    std::filesystem::path manifestPath = packagePath / "manifest.json";
    return ReadManifestFromFile(manifestPath);
}

bool AssetPackager::ExtractPackage(
    const std::filesystem::path& packagePath,
    const std::filesystem::path& destinationPath)
{
    if (!std::filesystem::exists(packagePath)) {
        Log::Error("Package path does not exist: " + packagePath.string());
        return false;
    }
    
    // Read manifest
    PackageManifest manifest = ReadPackageManifest(packagePath);
    if (manifest.packageName.empty()) {
        Log::Error("Invalid package manifest");
        return false;
    }
    
    // Create destination directory
    if (!std::filesystem::exists(destinationPath)) {
        std::filesystem::create_directories(destinationPath);
    }
    
    // Copy assets
    std::filesystem::path assetsSource = packagePath / "assets";
    if (std::filesystem::exists(assetsSource)) {
        std::filesystem::path assetsDest = destinationPath / "assets";
        try {
            std::filesystem::copy(assetsSource, assetsDest, 
                std::filesystem::copy_options::recursive | 
                std::filesystem::copy_options::overwrite_existing);
        } catch (const std::exception& e) {
            Log::Error("Failed to extract assets: " + std::string(e.what()));
            return false;
        }
    }
    
    // Copy manifest
    std::filesystem::path manifestSource = packagePath / "manifest.json";
    if (std::filesystem::exists(manifestSource)) {
        std::filesystem::path manifestDest = destinationPath / "manifest.json";
        std::filesystem::copy_file(manifestSource, manifestDest, 
            std::filesystem::copy_options::overwrite_existing);
    }
    
    Log::Info("Package extracted to: " + destinationPath.string());
    return true;
}

} // namespace LGE

