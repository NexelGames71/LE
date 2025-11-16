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

#include "LGE/core/filesystem/VirtualFileSystem.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace LGE {

VirtualFileSystem::VirtualFileSystem() {
}

VirtualFileSystem::~VirtualFileSystem() {
    Clear();
}

void VirtualFileSystem::Initialize(const std::filesystem::path& root) {
    m_ProjectRoot = root;
    Clear();
    
    // Auto-mount common directories
    Mount("/Assets", (root / "Assets").string());
    Mount("/Config", (root / "Config").string());
    Mount("/Intermediate", (root / "Config" / "Packages" / "Intermediate").string());
    Mount("/Saved", (root / "Config" / "Packages" / "Saved").string());
    
    Log::Info("Initialized VirtualFileSystem with root: " + root.string());
}

std::filesystem::path VirtualFileSystem::ResolveVirtualPath(const std::string& virtualPath) {
    std::string normalized = NormalizeVirtualPath(virtualPath);
    
    // Find matching mount point
    for (const auto& mount : m_MountPoints) {
        if (normalized.find(mount.virtualPath) == 0) {
            // Replace virtual path with physical path
            std::string remaining = normalized.substr(mount.virtualPath.length());
            if (remaining.empty() || remaining[0] == '/') {
                // Remove leading slash if present
                if (!remaining.empty() && remaining[0] == '/') {
                    remaining = remaining.substr(1);
                }
                return mount.physicalPath / remaining;
            }
        }
    }
    
    // If no mount point matches, return relative to project root
    std::string remaining = normalized;
    if (!remaining.empty() && remaining[0] == '/') {
        remaining = remaining.substr(1);
    }
    return m_ProjectRoot / remaining;
}

std::string VirtualFileSystem::GetVirtualPath(const std::filesystem::path& physicalPath) {
    std::filesystem::path normalized = std::filesystem::absolute(physicalPath);
    
    // Try to find matching mount point
    for (const auto& mount : m_MountPoints) {
        std::filesystem::path mountPath = std::filesystem::absolute(mount.physicalPath);
        
        // Check if physical path is within this mount point
        auto [it, end] = std::mismatch(normalized.begin(), normalized.end(), mountPath.begin(), mountPath.end());
        if (it == normalized.end() || (it == normalized.begin() && end == mountPath.end())) {
            // Path is within mount point
            std::filesystem::path relative = std::filesystem::relative(normalized, mountPath);
            std::string result = mount.virtualPath;
            if (!relative.empty() && relative != ".") {
                result += "/" + relative.string();
                // Replace backslashes with forward slashes
                std::replace(result.begin() + mount.virtualPath.length(), result.end(), '\\', '/');
            }
            return result;
        }
    }
    
    // If no mount point matches, return relative to project root
    std::filesystem::path relative = std::filesystem::relative(normalized, m_ProjectRoot);
    if (relative.empty() || relative == ".") {
        return "/";
    }
    std::string result = "/" + relative.string();
    std::replace(result.begin(), result.end(), '\\', '/');
    return result;
}

bool VirtualFileSystem::Mount(const std::string& virtualPath, const std::filesystem::path& physicalPath, bool readOnly) {
    std::string normalized = NormalizeVirtualPath(virtualPath);
    
    // Check if already mounted
    if (FindMountPoint(normalized)) {
        Log::Warn("Virtual path already mounted: " + normalized);
        return false;
    }
    
    // Verify physical path exists
    if (!std::filesystem::exists(physicalPath)) {
        Log::Warn("Physical path does not exist, creating: " + physicalPath.string());
        if (!std::filesystem::create_directories(physicalPath)) {
            Log::Error("Failed to create physical path: " + physicalPath.string());
            return false;
        }
    }
    
    m_MountPoints.emplace_back(normalized, std::filesystem::absolute(physicalPath), readOnly);
    Log::Info("Mounted: " + normalized + " -> " + physicalPath.string());
    return true;
}

bool VirtualFileSystem::Unmount(const std::string& virtualPath) {
    std::string normalized = NormalizeVirtualPath(virtualPath);
    
    auto it = std::remove_if(m_MountPoints.begin(), m_MountPoints.end(),
        [&normalized](const MountPoint& mp) { return mp.virtualPath == normalized; });
    
    if (it != m_MountPoints.end()) {
        m_MountPoints.erase(it, m_MountPoints.end());
        Log::Info("Unmounted: " + normalized);
        return true;
    }
    
    return false;
}

bool VirtualFileSystem::FileExists(const std::string& virtualPath) {
    std::filesystem::path physical = ResolveVirtualPath(virtualPath);
    return std::filesystem::exists(physical) && std::filesystem::is_regular_file(physical);
}

std::vector<uint8_t> VirtualFileSystem::ReadFile(const std::string& virtualPath) {
    std::filesystem::path physical = ResolveVirtualPath(virtualPath);
    
    if (!std::filesystem::exists(physical)) {
        Log::Error("File not found: " + virtualPath + " (resolved to: " + physical.string() + ")");
        return {};
    }
    
    std::ifstream file(physical, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Log::Error("Failed to open file: " + physical.string());
        return {};
    }
    
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    return data;
}

bool VirtualFileSystem::WriteFile(const std::string& virtualPath, const std::vector<uint8_t>& data) {
    std::filesystem::path physical = ResolveVirtualPath(virtualPath);
    
    // Check if mount point is read-only
    std::string normalized = NormalizeVirtualPath(virtualPath);
    const MountPoint* mount = FindMountPoint(normalized);
    if (mount && mount->readOnly) {
        Log::Error("Cannot write to read-only mount point: " + normalized);
        return false;
    }
    
    // Create parent directory if needed
    std::filesystem::create_directories(physical.parent_path());
    
    std::ofstream file(physical, std::ios::binary);
    if (!file.is_open()) {
        Log::Error("Failed to open file for writing: " + physical.string());
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return file.good();
}

bool VirtualFileSystem::DirectoryExists(const std::string& virtualPath) {
    std::filesystem::path physical = ResolveVirtualPath(virtualPath);
    return std::filesystem::exists(physical) && std::filesystem::is_directory(physical);
}

bool VirtualFileSystem::CreateDirectory(const std::string& virtualPath) {
    std::filesystem::path physical = ResolveVirtualPath(virtualPath);
    
    // Check if mount point is read-only
    std::string normalized = NormalizeVirtualPath(virtualPath);
    const MountPoint* mount = FindMountPoint(normalized);
    if (mount && mount->readOnly) {
        Log::Error("Cannot create directory in read-only mount point: " + normalized);
        return false;
    }
    
    return std::filesystem::create_directories(physical);
}

void VirtualFileSystem::Clear() {
    m_MountPoints.clear();
}

VirtualFileSystem::MountPoint* VirtualFileSystem::FindMountPoint(const std::string& virtualPath) {
    std::string normalized = NormalizeVirtualPath(virtualPath);
    
    for (auto& mount : m_MountPoints) {
        if (normalized.find(mount.virtualPath) == 0) {
            return &mount;
        }
    }
    
    return nullptr;
}

const VirtualFileSystem::MountPoint* VirtualFileSystem::FindMountPoint(const std::string& virtualPath) const {
    std::string normalized = NormalizeVirtualPath(virtualPath);
    
    for (const auto& mount : m_MountPoints) {
        if (normalized.find(mount.virtualPath) == 0) {
            return &mount;
        }
    }
    
    return nullptr;
}

std::string VirtualFileSystem::NormalizeVirtualPath(const std::string& path) const {
    std::string normalized = path;
    
    // Replace backslashes with forward slashes
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    
    // Ensure it starts with /
    if (normalized.empty() || normalized[0] != '/') {
        normalized = "/" + normalized;
    }
    
    // Remove trailing slash (except for root)
    if (normalized.length() > 1 && normalized.back() == '/') {
        normalized.pop_back();
    }
    
    return normalized;
}

} // namespace LGE

