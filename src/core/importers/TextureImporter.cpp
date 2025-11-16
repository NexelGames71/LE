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

#include "LGE/core/importers/TextureImporter.h"
#include "LGE/core/Log.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/rendering/Texture.h"
#include <algorithm>

// stb_image is already implemented in Texture.cpp, just include the header
#include "../../third_party/stb_image.h"

namespace LGE {

TextureImporter::TextureImporter(AssetRegistry* registry, VirtualFileSystem* vfs, GUIDRegistry* guidRegistry)
    : AssetImporter(registry)
    , m_VFS(vfs)
    , m_GUIDRegistry(guidRegistry)
{
}

std::vector<std::string> TextureImporter::GetSupportedExtensions() const {
    return {".png", ".jpg", ".jpeg", ".tga", ".bmp", ".dds", ".hdr", ".exr"};
}

AssetType TextureImporter::GetAssetType() const {
    return AssetType::Texture;
}

ImportSettings TextureImporter::GetDefaultSettings() const {
    ImportSettings settings;
    settings.Set("sRGB", true);
    settings.Set("generateMipmaps", true);
    settings.Set("compression", "None");
    settings.Set("maxSize", 2048);
    settings.Set("filterMode", "Trilinear");
    settings.Set("wrapMode", "Repeat");
    return settings;
}

bool TextureImporter::Import(
    const std::filesystem::path& sourcePath,
    const std::filesystem::path& destinationPath,
    const ImportSettings& settings,
    AssetMetadata& outMetadata
) {
    if (!std::filesystem::exists(sourcePath)) {
        Log::Error("Source file does not exist: " + sourcePath.string());
        return false;
    }
    
    // Load image
    int width, height, channels;
    unsigned char* data = stbi_load(
        sourcePath.string().c_str(),
        &width, &height, &channels, 4
    );
    
    if (!data) {
        Log::Error("Failed to load image: " + sourcePath.string());
        return false;
    }
    
    // Apply settings
    bool generateMips = settings.Get<bool>("generateMipmaps", true);
    int maxSize = settings.Get<int>("maxSize", 2048);
    
    // Resize if needed (simplified - would need proper image resizing library)
    int finalWidth = width;
    int finalHeight = height;
    if (width > maxSize || height > maxSize) {
        float scale = std::min(static_cast<float>(maxSize) / width, static_cast<float>(maxSize) / height);
        finalWidth = static_cast<int>(width * scale);
        finalHeight = static_cast<int>(height * scale);
        // Note: Actual resizing would require a library like stb_image_resize
        Log::Warn("Image resizing not fully implemented - using original size");
    }
    
    // Create destination directory if needed
    std::filesystem::create_directories(destinationPath.parent_path());
    
    // Save as .ltex (Luma texture format - for now, just copy the original)
    // In a full implementation, this would convert to an optimized format
    std::filesystem::path ltexPath = destinationPath;
    ltexPath.replace_extension(".ltex");
    
    // For now, just copy the file (in production, would convert to .ltex format)
    std::filesystem::copy_file(sourcePath, ltexPath, std::filesystem::copy_options::overwrite_existing);
    
    // Get virtual path
    std::string virtualPath = m_VFS ? m_VFS->GetVirtualPath(ltexPath) : ltexPath.string();
    
    // Create metadata
    outMetadata.guid = m_GUIDRegistry ? m_GUIDRegistry->GetOrCreateGUID(virtualPath) : GUID::Generate();
    outMetadata.type = AssetType::Texture;
    outMetadata.name = sourcePath.stem().string();
    outMetadata.virtualPath = virtualPath;
    outMetadata.importSettings = settings.ToJson();
    outMetadata.fileSize = std::filesystem::file_size(ltexPath);
    
    auto lastWriteTime = std::filesystem::last_write_time(ltexPath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        lastWriteTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    outMetadata.lastModified = std::chrono::system_clock::to_time_t(sctp);
    outMetadata.importDate = std::time(nullptr);
    
    stbi_image_free(data);
    
    Log::Info("Imported texture: " + sourcePath.string() + " -> " + ltexPath.string());
    return true;
}

bool TextureImporter::Reimport(const GUID& guid, const ImportSettings& settings) {
    if (!m_Registry) {
        Log::Error("Registry not set for TextureImporter");
        return false;
    }
    
    AssetMetadata* metadata = m_Registry->GetAsset(guid);
    if (!metadata) {
        Log::Error("Asset not found for GUID: " + guid.ToString());
        return false;
    }
    
    // Find source file (stored in .meta file or use current path)
    std::filesystem::path destPath = m_VFS ? m_VFS->ResolveVirtualPath(metadata->virtualPath) : metadata->virtualPath;
    
    // For reimport, we need the original source file
    // In a full implementation, this would be stored in the .meta file
    // For now, assume the source is the same as destination (without .ltex extension)
    std::filesystem::path sourcePath = destPath;
    sourcePath.replace_extension(".png"); // Try .png first
    
    if (!std::filesystem::exists(sourcePath)) {
        // Try other common extensions
        std::vector<std::string> extensions = {".jpg", ".jpeg", ".tga", ".bmp"};
        bool found = false;
        for (const auto& ext : extensions) {
            sourcePath.replace_extension(ext);
            if (std::filesystem::exists(sourcePath)) {
                found = true;
                break;
            }
        }
        if (!found) {
            Log::Error("Source file not found for reimport: " + destPath.string());
            return false;
        }
    }
    
    AssetMetadata newMetadata;
    if (Import(sourcePath, destPath, settings, newMetadata)) {
        // Update existing metadata
        newMetadata.guid = guid; // Keep same GUID
        m_Registry->UpdateAsset(guid, newMetadata);
        return true;
    }
    
    return false;
}

} // namespace LGE

