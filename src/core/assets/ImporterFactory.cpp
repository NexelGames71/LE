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

#include "LGE/core/assets/ImporterFactory.h"
#include "LGE/core/Log.h"
#include <algorithm>
#include <cctype>

namespace LGE {

ImporterFactory::ImporterFactory() {
}

ImporterFactory::~ImporterFactory() {
}

void ImporterFactory::RegisterImporter(std::unique_ptr<AssetImporter> importer) {
    if (!importer) {
        Log::Warn("Attempted to register null importer");
        return;
    }
    
    // Get type and extensions before releasing
    AssetType type = importer->GetAssetType();
    std::vector<std::string> extensions = importer->GetSupportedExtensions();
    AssetImporter* importerPtr = importer.get();
    
    // Convert to shared_ptr for shared ownership across extensions
    std::shared_ptr<AssetImporter> sharedImporter(importer.release());
    
    // Register by extensions (all share the same shared_ptr)
    for (const auto& ext : extensions) {
        std::string lowerExt = ext;
        std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
        m_Importers[lowerExt] = sharedImporter;
    }
    
    // Register by type
    m_TypeToImporter[type] = sharedImporter.get();
    
    // Create temporary metadata to get type name
    AssetMetadata temp;
    temp.type = type;
    Log::Info("Registered importer for type: " + temp.GetTypeName());
}

AssetImporter* ImporterFactory::GetImporterForExtension(const std::string& extension) {
    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    
    // Ensure extension starts with .
    if (!lowerExt.empty() && lowerExt[0] != '.') {
        lowerExt = "." + lowerExt;
    }
    
    auto it = m_Importers.find(lowerExt);
    if (it != m_Importers.end()) {
        return it->second.get();
    }
    
    return nullptr;
}

AssetImporter* ImporterFactory::GetImporterForType(AssetType type) {
    auto it = m_TypeToImporter.find(type);
    if (it != m_TypeToImporter.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> ImporterFactory::GetAllSupportedExtensions() const {
    std::vector<std::string> extensions;
    for (const auto& [ext, importer] : m_Importers) {
        extensions.push_back(ext);
    }
    return extensions;
}

bool ImporterFactory::IsExtensionSupported(const std::string& extension) const {
    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    
    if (!lowerExt.empty() && lowerExt[0] != '.') {
        lowerExt = "." + lowerExt;
    }
    
    return m_Importers.find(lowerExt) != m_Importers.end();
}

std::vector<AssetImporter*> ImporterFactory::GetAllImporters() const {
    std::vector<AssetImporter*> importers;
    std::vector<AssetImporter*> seen;
    for (const auto& [ext, importer] : m_Importers) {
        AssetImporter* ptr = importer.get();
        if (std::find(seen.begin(), seen.end(), ptr) == seen.end()) {
            importers.push_back(ptr);
            seen.push_back(ptr);
        }
    }
    return importers;
}

} // namespace LGE

