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

#include "LGE/core/assets/MetaFile.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <sstream>

namespace LGE {

MetaFile::MetaFile(const std::filesystem::path& asset)
    : m_AssetPath(asset)
    , m_MetaPath(GetMetaPathForAsset(asset))
    , sourceFileModTime(0)
{
}

bool MetaFile::Exists() const {
    return std::filesystem::exists(m_MetaPath);
}

bool MetaFile::Load() {
    if (!Exists()) {
        return false;
    }
    
    std::string json = FileSystem::ReadFile(m_MetaPath.string());
    if (json.empty()) {
        Log::Error("Failed to read meta file: " + m_MetaPath.string());
        return false;
    }
    
    // Parse GUID
    size_t guidPos = json.find("\"guid\"");
    if (guidPos != std::string::npos) {
        size_t colonPos = json.find(':', guidPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                std::string guidStr = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                guid = GUID::FromString(guidStr);
            }
        }
    }
    
    // Parse sourceFile
    size_t sourcePos = json.find("\"sourceFile\"");
    if (sourcePos != std::string::npos) {
        size_t colonPos = json.find(':', sourcePos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                sourceFile = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Parse importSettings
    size_t settingsPos = json.find("\"importSettings\"");
    if (settingsPos != std::string::npos) {
        size_t objStart = json.find('{', settingsPos);
        size_t objEnd = json.find('}', objStart);
        if (objStart != std::string::npos && objEnd != std::string::npos) {
            std::string settingsJson = json.substr(objStart, objEnd - objStart + 1);
            importSettings.FromJson(settingsJson);
        }
    }
    
    // Parse dependencies
    size_t depsPos = json.find("\"dependencies\"");
    if (depsPos != std::string::npos) {
        size_t arrayStart = json.find('[', depsPos);
        size_t arrayEnd = json.find(']', arrayStart);
        if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
            std::string depsArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            size_t pos = 0;
            while ((pos = depsArray.find('"', pos)) != std::string::npos) {
                size_t quoteStart = pos;
                size_t quoteEnd = depsArray.find('"', quoteStart + 1);
                if (quoteEnd != std::string::npos) {
                    std::string guidStr = depsArray.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                    GUID depGuid = GUID::FromString(guidStr);
                    if (depGuid.IsValid()) {
                        dependencies.push_back(depGuid);
                    }
                    pos = quoteEnd + 1;
                } else {
                    break;
                }
            }
        }
    }
    
    // Parse sourceModTime
    size_t modTimePos = json.find("\"sourceModTime\"");
    if (modTimePos != std::string::npos) {
        size_t colonPos = json.find(':', modTimePos);
        size_t numStart = json.find_first_of("0123456789", colonPos);
        if (numStart != std::string::npos) {
            size_t numEnd = json.find_first_not_of("0123456789", numStart);
            if (numEnd == std::string::npos) numEnd = json.length();
            try {
                sourceFileModTime = std::stoll(json.substr(numStart, numEnd - numStart));
            } catch (...) {}
        }
    }
    
    return true;
}

bool MetaFile::Save() {
    std::ostringstream json;
    json << "{\n";
    json << "  \"guid\": \"" << guid.ToString() << "\",\n";
    json << "  \"sourceFile\": \"" << sourceFile << "\",\n";
    json << "  \"importSettings\": " << importSettings.ToJson() << ",\n";
    
    // Dependencies
    json << "  \"dependencies\": [\n";
    for (size_t i = 0; i < dependencies.size(); ++i) {
        json << "    \"" << dependencies[i].ToString() << "\"";
        if (i < dependencies.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";
    
    // Source mod time
    json << "  \"sourceModTime\": " << sourceFileModTime << "\n";
    json << "}\n";
    
    if (!FileSystem::WriteFile(m_MetaPath.string(), json.str())) {
        Log::Error("Failed to write meta file: " + m_MetaPath.string());
        return false;
    }
    
    return true;
}

} // namespace LGE



