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

#include "LGE/core/assets/AssetImporter.h"
#include "LGE/core/assets/AssetMetadata.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

namespace LGE {

class ImporterFactory {
private:
    std::unordered_map<std::string, std::shared_ptr<AssetImporter>> m_Importers;
    std::unordered_map<AssetType, AssetImporter*> m_TypeToImporter;

public:
    ImporterFactory();
    ~ImporterFactory();
    
    // Register an importer
    void RegisterImporter(std::unique_ptr<AssetImporter> importer);
    
    // Get importer for file extension
    AssetImporter* GetImporterForExtension(const std::string& extension);
    
    // Get importer for asset type
    AssetImporter* GetImporterForType(AssetType type);
    
    // Get all supported extensions
    std::vector<std::string> GetAllSupportedExtensions() const;
    
    // Check if extension is supported
    bool IsExtensionSupported(const std::string& extension) const;
    
    // Get all registered importers
    std::vector<AssetImporter*> GetAllImporters() const;
};

} // namespace LGE

