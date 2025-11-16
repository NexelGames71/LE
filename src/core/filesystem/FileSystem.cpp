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

#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
// Undefine Windows macros that conflict with our methods
#ifdef CreateDirectory
#undef CreateDirectory
#endif
#ifdef DeleteFile
#undef DeleteFile
#endif
#ifdef CopyFile
#undef CopyFile
#endif
#ifdef MoveFile
#undef MoveFile
#endif
#else
#include <unistd.h>
#endif

namespace LGE {

std::string FileSystem::ReadFile(const std::string& filepath) {
    // Try multiple paths: relative to current directory, relative to executable, and absolute
    std::vector<std::string> pathsToTry;
    
    // First try the path as-is (relative to current working directory)
    pathsToTry.push_back(filepath);
    
    // Try relative to executable directory (for when running from build/bin/Release/)
    #ifdef _WIN32
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
    // Go up from build/bin/Release/ to project root
    std::filesystem::path projectRoot = exeDir.parent_path().parent_path().parent_path();
    pathsToTry.push_back((projectRoot / filepath).string());
    #else
    // For Linux/Mac, try relative to executable
    char exePath[1024];
    ssize_t count = readlink("/proc/self/exe", exePath, 1024);
    if (count != -1) {
        std::filesystem::path exeDir = std::filesystem::path(std::string(exePath, count)).parent_path();
        std::filesystem::path projectRoot = exeDir.parent_path().parent_path().parent_path();
        pathsToTry.push_back((projectRoot / filepath).string());
    }
    #endif
    
    for (const auto& path : pathsToTry) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
    }
    
    Log::Error("Failed to open file: " + filepath);
    return "";
}

bool FileSystem::WriteFile(const std::string& filepath, const std::string& content) {
    try {
        // Create parent directory if it doesn't exist
        std::filesystem::path path(filepath);
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            Log::Error("Failed to write file: " + filepath);
            return false;
        }
        
        file << content;
        file.close();
        return true;
    } catch (const std::exception& e) {
        Log::Error("Exception writing file: " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::Exists(const std::string& path) {
    return std::filesystem::exists(path);
}

bool FileSystem::IsDirectory(const std::string& path) {
    return std::filesystem::is_directory(path);
}

bool FileSystem::IsFile(const std::string& path) {
    return std::filesystem::is_regular_file(path);
}

bool FileSystem::CreateDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception& e) {
        Log::Error("Failed to create directory: " + path + " - " + e.what());
        return false;
    }
}

bool FileSystem::DeleteFile(const std::string& path) {
    try {
        return std::filesystem::remove(path);
    } catch (const std::exception& e) {
        Log::Error("Failed to delete file: " + path + " - " + e.what());
        return false;
    }
}

bool FileSystem::DeleteDirectory(const std::string& path) {
    try {
        return std::filesystem::remove_all(path) > 0;
    } catch (const std::exception& e) {
        Log::Error("Failed to delete directory: " + path + " - " + e.what());
        return false;
    }
}

bool FileSystem::CopyFile(const std::string& src, const std::string& dst) {
    try {
        std::filesystem::copy(src, dst, std::filesystem::copy_options::recursive);
        return true;
    } catch (const std::exception& e) {
        Log::Error("Failed to copy file: " + src + " to " + dst + " - " + e.what());
        return false;
    }
}

bool FileSystem::MoveFile(const std::string& src, const std::string& dst) {
    try {
        std::filesystem::rename(src, dst);
        return true;
    } catch (const std::exception& e) {
        Log::Error("Failed to move file: " + src + " to " + dst + " - " + e.what());
        return false;
    }
}

std::vector<FileEntry> FileSystem::ListDirectory(const std::string& path, bool recursive) {
    std::vector<FileEntry> entries;
    
    try {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
            return entries;
        }
        
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                FileEntry fileEntry;
                fileEntry.path = entry.path().string();
                fileEntry.name = entry.path().filename().string();
                fileEntry.isDirectory = entry.is_directory();
                
                if (entry.is_regular_file()) {
                    fileEntry.extension = entry.path().extension().string();
                    fileEntry.fileType = GetFileTypeFromExtension(fileEntry.extension);
                    fileEntry.fileSize = entry.file_size();
                    
                    // Convert file time to time_t
                    auto fileTime = entry.last_write_time();
                    auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        fileTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
                    fileEntry.lastModified = std::chrono::system_clock::to_time_t(systemTime);
                } else {
                    fileEntry.fileType = EFileType::Unknown;
                    fileEntry.fileSize = 0;
                    fileEntry.lastModified = 0;
                }
                
                entries.push_back(fileEntry);
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                FileEntry fileEntry;
                fileEntry.path = entry.path().string();
                fileEntry.name = entry.path().filename().string();
                fileEntry.isDirectory = entry.is_directory();
                
                if (entry.is_regular_file()) {
                    fileEntry.extension = entry.path().extension().string();
                    fileEntry.fileType = GetFileTypeFromExtension(fileEntry.extension);
                    fileEntry.fileSize = entry.file_size();
                    
                    // Convert file time to time_t
                    auto fileTime = entry.last_write_time();
                    auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        fileTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
                    fileEntry.lastModified = std::chrono::system_clock::to_time_t(systemTime);
                } else {
                    fileEntry.fileType = EFileType::Unknown;
                    fileEntry.fileSize = 0;
                    fileEntry.lastModified = 0;
                }
                
                entries.push_back(fileEntry);
            }
        }
    } catch (const std::exception& e) {
        Log::Error("Failed to list directory: " + path + " - " + e.what());
    }
    
    return entries;
}

std::vector<std::string> FileSystem::GetDirectories(const std::string& path) {
    std::vector<std::string> directories;
    
    try {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
            return directories;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                directories.push_back(entry.path().string());
            }
        }
    } catch (const std::exception& e) {
        Log::Error("Failed to get directories: " + path + " - " + e.what());
    }
    
    return directories;
}

std::vector<std::string> FileSystem::GetFiles(const std::string& path, const std::string& extension) {
    std::vector<std::string> files;
    
    try {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
            return files;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                if (extension.empty() || entry.path().extension() == extension) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception& e) {
        Log::Error("Failed to get files: " + path + " - " + e.what());
    }
    
    return files;
}

std::string FileSystem::GetFileName(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

std::string FileSystem::GetFileNameWithoutExtension(const std::string& path) {
    return std::filesystem::path(path).stem().string();
}

std::string FileSystem::GetExtension(const std::string& path) {
    return std::filesystem::path(path).extension().string();
}

std::string FileSystem::GetDirectory(const std::string& path) {
    return std::filesystem::path(path).parent_path().string();
}

std::string FileSystem::GetParentDirectory(const std::string& path) {
    return std::filesystem::path(path).parent_path().parent_path().string();
}

std::string FileSystem::NormalizePath(const std::string& path) {
    try {
        std::filesystem::path normalized = std::filesystem::canonical(path);
        std::string result = normalized.string();
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    } catch (...) {
        // If canonical fails (e.g., path doesn't exist), just normalize separators
        std::string result = path;
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }
}

std::string FileSystem::JoinPath(const std::string& path1, const std::string& path2) {
    return (std::filesystem::path(path1) / path2).string();
}

std::string FileSystem::GetAbsolutePath(const std::string& path) {
    try {
        return std::filesystem::absolute(path).string();
    } catch (...) {
        return path;
    }
}

std::string FileSystem::GetRelativePath(const std::string& path, const std::string& base) {
    try {
        return std::filesystem::relative(path, base).string();
    } catch (...) {
        return path;
    }
}

std::vector<FileEntry> FileSystem::FindAssets(const std::string& rootPath, EFileType fileType) {
    std::vector<FileEntry> assets;
    auto entries = ListDirectory(rootPath, true);
    
    for (const auto& entry : entries) {
        if (!entry.isDirectory && entry.fileType == fileType) {
            assets.push_back(entry);
        }
    }
    
    return assets;
}

std::vector<FileEntry> FileSystem::FindAssetsByExtension(const std::string& rootPath, const std::string& extension) {
    std::vector<FileEntry> assets;
    auto entries = ListDirectory(rootPath, true);
    
    std::string extLower = extension;
    std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);
    
    for (const auto& entry : entries) {
        if (!entry.isDirectory) {
            std::string entryExt = entry.extension;
            std::transform(entryExt.begin(), entryExt.end(), entryExt.begin(), ::tolower);
            
            if (entryExt == extLower) {
                assets.push_back(entry);
            }
        }
    }
    
    return assets;
}

EFileType FileSystem::GetFileType(const std::string& filepath) {
    return GetFileTypeFromExtension(GetExtension(filepath));
}

EFileType FileSystem::GetFileTypeFromExtension(const std::string& extension) {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // Materials
    if (ext == ".material") return EFileType::Material;
    
    // Textures
    if (ext == ".texture" || ext == ".png" || ext == ".jpg" || ext == ".jpeg" || 
        ext == ".tga" || ext == ".bmp" || ext == ".hdr" || ext == ".exr") {
        return EFileType::Texture;
    }
    
    // Shaders
    if (ext == ".vert" || ext == ".frag" || ext == ".geom" || ext == ".compute" || 
        ext == ".glsl" || ext == ".hlsl") {
        return EFileType::Shader;
    }
    
    // Models
    if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb" || 
        ext == ".dae" || ext == ".3ds") {
        return EFileType::Model;
    }
    
    // Scenes
    if (ext == ".scene" || ext == ".lscene") return EFileType::Scene;
    
    // Scripts
    if (ext == ".lua" || ext == ".py") return EFileType::Script;
    
    // Audio
    if (ext == ".wav" || ext == ".ogg" || ext == ".mp3" || ext == ".flac") {
        return EFileType::Audio;
    }
    
    // Fonts
    if (ext == ".ttf" || ext == ".otf") return EFileType::Font;
    
    // Config
    if (ext == ".json" || ext == ".yaml" || ext == ".yml" || ext == ".ini") {
        return EFileType::Config;
    }
    
    return EFileType::Unknown;
}

std::string FileSystem::GetProjectRoot() {
    #ifdef _WIN32
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
    // Go up from build/bin/Release/ to project root
    return exeDir.parent_path().parent_path().parent_path().string();
    #else
    char exePath[1024];
    ssize_t count = readlink("/proc/self/exe", exePath, 1024);
    if (count != -1) {
        std::filesystem::path exeDir = std::filesystem::path(std::string(exePath, count)).parent_path();
        return exeDir.parent_path().parent_path().parent_path().string();
    }
    return ".";
    #endif
}

std::string FileSystem::GetAssetsDirectory() {
    return JoinPath(GetProjectRoot(), "assets");
}

std::string FileSystem::GetMaterialsDirectory() {
    return JoinPath(GetAssetsDirectory(), "materials");
}

std::string FileSystem::GetTexturesDirectory() {
    return JoinPath(GetAssetsDirectory(), "textures");
}

std::string FileSystem::GetShadersDirectory() {
    return JoinPath(GetAssetsDirectory(), "shaders");
}

std::string FileSystem::GetScenesDirectory() {
    return JoinPath(GetAssetsDirectory(), "scenes");
}

void FileSystem::WatchDirectory(const std::string& path, std::function<void(const std::string&)> callback) {
    // TODO: Implement file watching (use platform-specific APIs or a library)
    Log::Warn("FileSystem::WatchDirectory not yet implemented");
}

void FileSystem::StopWatching(const std::string& path) {
    // TODO: Implement file watching
    Log::Warn("FileSystem::StopWatching not yet implemented");
}

} // namespace LGE

