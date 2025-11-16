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

#include "LGE/core/assets/AssetMetadata.h"
#include <sstream>
#include <algorithm>
#include <ctime>

namespace LGE {

AssetMetadata::AssetMetadata()
    : guid(GUID::Invalid())
    , type(AssetType::Unknown)
    , fileSize(0)
    , lastModified(0)
    , importDate(std::time(nullptr))
{
}

std::string AssetMetadata::ToJson() const {
    std::ostringstream json;
    json << "{\n";
    
    json << "  \"guid\": \"" << guid.ToString() << "\",\n";
    json << "  \"virtualPath\": \"" << virtualPath << "\",\n";
    json << "  \"type\": \"" << GetTypeName() << "\",\n";
    json << "  \"name\": \"" << name << "\",\n";
    json << "  \"fileSize\": " << fileSize << ",\n";
    json << "  \"lastModified\": " << lastModified << ",\n";
    json << "  \"importDate\": " << importDate << ",\n";
    
    // Import settings
    json << "  \"importSettings\": " << (importSettings.empty() ? "{}" : importSettings) << ",\n";
    
    // Dependencies
    json << "  \"dependencies\": [\n";
    for (size_t i = 0; i < dependencies.size(); ++i) {
        json << "    \"" << dependencies[i].ToString() << "\"";
        if (i < dependencies.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";
    
    // Tags
    json << "  \"tags\": [\n";
    for (size_t i = 0; i < tags.size(); ++i) {
        json << "    \"" << tags[i] << "\"";
        if (i < tags.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";
    
    // Category
    json << "  \"category\": \"" << category << "\",\n";
    
    // Thumbnail
    json << "  \"thumbnailPath\": \"" << thumbnailPath << "\"\n";
    
    json << "}\n";
    
    return json.str();
}

AssetMetadata AssetMetadata::FromJson(const std::string& json) {
    AssetMetadata metadata;
    
    // Parse GUID
    size_t guidPos = json.find("\"guid\"");
    if (guidPos != std::string::npos) {
        size_t colonPos = json.find(':', guidPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                std::string guidStr = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                metadata.guid = GUID::FromString(guidStr);
            }
        }
    }
    
    // Parse virtualPath
    size_t pathPos = json.find("\"virtualPath\"");
    if (pathPos != std::string::npos) {
        size_t colonPos = json.find(':', pathPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                metadata.virtualPath = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Parse type
    size_t typePos = json.find("\"type\"");
    if (typePos != std::string::npos) {
        size_t colonPos = json.find(':', typePos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                std::string typeStr = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                metadata.type = GetTypeFromName(typeStr);
            }
        }
    }
    
    // Parse name
    size_t namePos = json.find("\"name\"");
    if (namePos != std::string::npos) {
        size_t colonPos = json.find(':', namePos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                metadata.name = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Parse fileSize
    size_t sizePos = json.find("\"fileSize\"");
    if (sizePos != std::string::npos) {
        size_t colonPos = json.find(':', sizePos);
        size_t numStart = json.find_first_of("0123456789", colonPos);
        if (numStart != std::string::npos) {
            size_t numEnd = json.find_first_not_of("0123456789", numStart);
            if (numEnd == std::string::npos) numEnd = json.length();
            metadata.fileSize = std::stoull(json.substr(numStart, numEnd - numStart));
        }
    }
    
    // Parse lastModified
    size_t modPos = json.find("\"lastModified\"");
    if (modPos != std::string::npos) {
        size_t colonPos = json.find(':', modPos);
        size_t numStart = json.find_first_of("0123456789", colonPos);
        if (numStart != std::string::npos) {
            size_t numEnd = json.find_first_not_of("0123456789", numStart);
            if (numEnd == std::string::npos) numEnd = json.length();
            metadata.lastModified = std::stoll(json.substr(numStart, numEnd - numStart));
        }
    }
    
    // Parse importDate
    size_t importPos = json.find("\"importDate\"");
    if (importPos != std::string::npos) {
        size_t colonPos = json.find(':', importPos);
        size_t numStart = json.find_first_of("0123456789", colonPos);
        if (numStart != std::string::npos) {
            size_t numEnd = json.find_first_not_of("0123456789", numStart);
            if (numEnd == std::string::npos) numEnd = json.length();
            metadata.importDate = std::stoll(json.substr(numStart, numEnd - numStart));
        }
    }
    
    // Parse dependencies array
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
                        metadata.dependencies.push_back(depGuid);
                    }
                    pos = quoteEnd + 1;
                } else {
                    break;
                }
            }
        }
    }
    
    // Parse tags array
    size_t tagsPos = json.find("\"tags\"");
    if (tagsPos != std::string::npos) {
        size_t arrayStart = json.find('[', tagsPos);
        size_t arrayEnd = json.find(']', arrayStart);
        if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
            std::string tagsArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            size_t pos = 0;
            while ((pos = tagsArray.find('"', pos)) != std::string::npos) {
                size_t quoteStart = pos;
                size_t quoteEnd = tagsArray.find('"', quoteStart + 1);
                if (quoteEnd != std::string::npos) {
                    std::string tag = tagsArray.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                    if (!tag.empty()) {
                        metadata.tags.push_back(tag);
                    }
                    pos = quoteEnd + 1;
                } else {
                    break;
                }
            }
        }
    }
    
    // Parse category
    size_t catPos = json.find("\"category\"");
    if (catPos != std::string::npos) {
        size_t colonPos = json.find(':', catPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                metadata.category = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Parse thumbnailPath
    size_t thumbPos = json.find("\"thumbnailPath\"");
    if (thumbPos != std::string::npos) {
        size_t colonPos = json.find(':', thumbPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                metadata.thumbnailPath = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    return metadata;
}

std::string AssetMetadata::GetTypeName() const {
    switch (type) {
        case AssetType::Texture: return "Texture";
        case AssetType::Model: return "Model";
        case AssetType::Material: return "Material";
        case AssetType::Shader: return "Shader";
        case AssetType::Script: return "Script";
        case AssetType::Audio: return "Audio";
        case AssetType::Scene: return "Scene";
        case AssetType::Prefab: return "Prefab";
        case AssetType::Animation: return "Animation";
        case AssetType::Font: return "Font";
        default: return "Unknown";
    }
}

AssetType AssetMetadata::GetTypeFromName(const std::string& typeName) {
    if (typeName == "Texture") return AssetType::Texture;
    if (typeName == "Model") return AssetType::Model;
    if (typeName == "Material") return AssetType::Material;
    if (typeName == "Shader") return AssetType::Shader;
    if (typeName == "Script") return AssetType::Script;
    if (typeName == "Audio") return AssetType::Audio;
    if (typeName == "Scene") return AssetType::Scene;
    if (typeName == "Prefab") return AssetType::Prefab;
    if (typeName == "Animation") return AssetType::Animation;
    if (typeName == "Font") return AssetType::Font;
    return AssetType::Unknown;
}

} // namespace LGE



