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

#include <string>
#include <vector>
#include <functional>

namespace LGE {

// File types for asset classification
enum class EFileType {
    Unknown,
    Material,      // .material
    Texture,       // .texture, .png, .jpg, .hdr, .exr
    Shader,        // .vert, .frag, .compute, .glsl
    Model,         // .obj, .fbx, .gltf
    Scene,         // .scene
    Script,        // .lua, .py
    Audio,         // .wav, .ogg, .mp3
    Font,          // .ttf, .otf
    Config         // .json, .yaml, .ini
};

// File entry information
struct FileEntry {
    std::string path;
    std::string name;
    std::string extension;
    bool isDirectory;
    EFileType fileType;
    size_t fileSize;
    uint64_t lastModified;  // Timestamp
};

class FileSystem {
public:
    // File reading
    static std::string ReadFile(const std::string& filepath);
    static bool WriteFile(const std::string& filepath, const std::string& content);
    
    // File operations
    static bool Exists(const std::string& path);
    static bool IsDirectory(const std::string& path);
    static bool IsFile(const std::string& path);
    static bool CreateDirectory(const std::string& path);
    static bool DeleteFile(const std::string& path);
    static bool DeleteDirectory(const std::string& path);
    static bool CopyFile(const std::string& src, const std::string& dst);
    static bool MoveFile(const std::string& src, const std::string& dst);
    
    // Directory operations
    static std::vector<FileEntry> ListDirectory(const std::string& path, bool recursive = false);
    static std::vector<std::string> GetDirectories(const std::string& path);
    static std::vector<std::string> GetFiles(const std::string& path, const std::string& extension = "");
    
    // Path utilities
    static std::string GetFileName(const std::string& path);
    static std::string GetFileNameWithoutExtension(const std::string& path);
    static std::string GetExtension(const std::string& path);
    static std::string GetDirectory(const std::string& path);
    static std::string GetParentDirectory(const std::string& path);
    static std::string NormalizePath(const std::string& path);
    static std::string JoinPath(const std::string& path1, const std::string& path2);
    static std::string GetAbsolutePath(const std::string& path);
    static std::string GetRelativePath(const std::string& path, const std::string& base);
    
    // Asset discovery
    static std::vector<FileEntry> FindAssets(const std::string& rootPath, EFileType fileType);
    static std::vector<FileEntry> FindAssetsByExtension(const std::string& rootPath, const std::string& extension);
    static EFileType GetFileType(const std::string& filepath);
    static EFileType GetFileTypeFromExtension(const std::string& extension);
    
    // Project structure
    static std::string GetProjectRoot();
    static std::string GetAssetsDirectory();
    static std::string GetMaterialsDirectory();
    static std::string GetTexturesDirectory();
    static std::string GetShadersDirectory();
    static std::string GetScenesDirectory();
    
    // File watching (for future use)
    static void WatchDirectory(const std::string& path, std::function<void(const std::string&)> callback);
    static void StopWatching(const std::string& path);
};

} // namespace LGE

