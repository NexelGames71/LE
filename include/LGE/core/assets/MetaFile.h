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
#include "LGE/core/ImportSettings.h"
#include <filesystem>
#include <vector>
#include <ctime>

namespace LGE {

class MetaFile {
private:
    std::filesystem::path m_AssetPath;
    std::filesystem::path m_MetaPath;

public:
    GUID guid;
    ImportSettings importSettings;
    std::vector<GUID> dependencies;
    std::time_t sourceFileModTime;
    std::string sourceFile; // Original source file path
    
    MetaFile(const std::filesystem::path& asset);
    
    bool Exists() const;
    bool Load();
    bool Save();
    
    std::filesystem::path GetMetaFilePath() const {
        return m_MetaPath;
    }
    
    static std::filesystem::path GetMetaPathForAsset(const std::filesystem::path& asset) {
        return std::filesystem::path(asset.string() + ".meta");
    }
    
    std::filesystem::path GetAssetPath() const { return m_AssetPath; }
};

} // namespace LGE



