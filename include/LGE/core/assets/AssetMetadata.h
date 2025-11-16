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
#include <ctime>
#include <map>

namespace LGE {

enum class AssetType {
    Unknown,
    Texture,
    Model,
    Material,
    Shader,
    Script,
    Audio,
    Scene,
    Prefab,
    Animation,
    Font
};

struct AssetMetadata {
    GUID guid;
    std::string virtualPath;
    AssetType type;
    std::string name;
    uint64_t fileSize;
    std::time_t lastModified;
    std::time_t importDate;
    
    // Import settings (key-value pairs stored as JSON string)
    std::string importSettings; // JSON string
    
    // Dependencies
    std::vector<GUID> dependencies;
    
    // Tags and search metadata
    std::vector<std::string> tags;
    std::string category;
    
    // Thumbnail
    std::string thumbnailPath;
    
    // Default constructor
    AssetMetadata();
    
    // Serialization
    std::string ToJson() const;
    static AssetMetadata FromJson(const std::string& json);
    
    // Helper methods
    std::string GetTypeName() const;
    static AssetType GetTypeFromName(const std::string& typeName);
};

} // namespace LGE



