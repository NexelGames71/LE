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

#include "LGE/core/assets/AssetMetadata.h"
#include "LGE/core/ImportSettings.h"
#include "LGE/core/assets/AssetRegistry.h"
#include <filesystem>
#include <vector>
#include <string>

namespace LGE {

class AssetImporter {
protected:
    AssetRegistry* m_Registry;

public:
    AssetImporter(AssetRegistry* registry) : m_Registry(registry) {}
    virtual ~AssetImporter() = default;
    
    // Get supported file extensions
    virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    
    // Get asset type this importer produces
    virtual AssetType GetAssetType() const = 0;
    
    // Get default import settings
    virtual ImportSettings GetDefaultSettings() const = 0;
    
    // Show import settings dialog (optional - returns true if user confirmed)
    virtual bool ShowImportDialog(ImportSettings& settings) { return true; }
    
    // Perform the import
    virtual bool Import(
        const std::filesystem::path& sourcePath,
        const std::filesystem::path& destinationPath,
        const ImportSettings& settings,
        AssetMetadata& outMetadata
    ) = 0;
    
    // Reimport existing asset
    virtual bool Reimport(const GUID& guid, const ImportSettings& settings) = 0;
    
    // Set registry
    void SetRegistry(AssetRegistry* registry) { m_Registry = registry; }
};

} // namespace LGE



