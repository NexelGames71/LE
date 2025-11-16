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
#include "LGE/core/filesystem/VirtualFileSystem.h"
#include "LGE/core/GUIDRegistry.h"

namespace LGE {

class TextureImporter : public AssetImporter {
private:
    VirtualFileSystem* m_VFS;
    GUIDRegistry* m_GUIDRegistry;

public:
    TextureImporter(AssetRegistry* registry, VirtualFileSystem* vfs, GUIDRegistry* guidRegistry);
    
    std::vector<std::string> GetSupportedExtensions() const override;
    AssetType GetAssetType() const override;
    ImportSettings GetDefaultSettings() const override;
    
    bool Import(
        const std::filesystem::path& sourcePath,
        const std::filesystem::path& destinationPath,
        const ImportSettings& settings,
        AssetMetadata& outMetadata
    ) override;
    
    bool Reimport(const GUID& guid, const ImportSettings& settings) override;
};

} // namespace LGE



