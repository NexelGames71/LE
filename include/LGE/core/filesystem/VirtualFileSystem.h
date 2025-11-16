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
#include <filesystem>
#include <memory>

namespace LGE {

class VirtualFileSystem {
private:
    struct MountPoint {
        std::string virtualPath;   // "/Assets"
        std::filesystem::path physicalPath; // "C:/LumaProject/Assets"
        bool readOnly;
        
        MountPoint(const std::string& vPath, const std::filesystem::path& pPath, bool ro = false)
            : virtualPath(vPath), physicalPath(pPath), readOnly(ro) {}
    };
    
    std::vector<MountPoint> m_MountPoints;
    std::filesystem::path m_ProjectRoot;

public:
    VirtualFileSystem();
    ~VirtualFileSystem();
    
    // Initialize VFS with project root
    void Initialize(const std::filesystem::path& root);
    
    // Convert virtual to physical path
    std::filesystem::path ResolveVirtualPath(const std::string& virtualPath);
    
    // Convert physical to virtual path
    std::string GetVirtualPath(const std::filesystem::path& physicalPath);
    
    // Mount a directory
    bool Mount(const std::string& virtualPath, const std::filesystem::path& physicalPath, bool readOnly = false);
    
    // Unmount a directory
    bool Unmount(const std::string& virtualPath);
    
    // File operations using virtual paths
    bool FileExists(const std::string& virtualPath);
    std::vector<uint8_t> ReadFile(const std::string& virtualPath);
    bool WriteFile(const std::string& virtualPath, const std::vector<uint8_t>& data);
    
    // Directory operations
    bool DirectoryExists(const std::string& virtualPath);
    bool CreateDirectory(const std::string& virtualPath);
    
    // Get all mount points
    const std::vector<MountPoint>& GetMountPoints() const { return m_MountPoints; }
    
    // Clear all mount points
    void Clear();

private:
    // Helper methods
    MountPoint* FindMountPoint(const std::string& virtualPath);
    const MountPoint* FindMountPoint(const std::string& virtualPath) const;
    std::string NormalizeVirtualPath(const std::string& path) const;
};

} // namespace LGE

