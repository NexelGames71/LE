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

#include "LGE/core/filesystem/FileSystem.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <functional>

namespace LGE {

// Forward declarations
class Texture;

// Directory tree node for folder hierarchy
struct DirectoryNode {
    std::string path;
    std::string name;
    std::vector<std::shared_ptr<DirectoryNode>> children;
    bool isExpanded;
    bool hasScanned;
    
    DirectoryNode(const std::string& p, const std::string& n) 
        : path(p), name(n), isExpanded(false), hasScanned(false) {}
};

class ContentBrowser {
public:
    ContentBrowser();
    ~ContentBrowser();

    void OnUIRender();
    
    void RefreshAssets();
    void RefreshDirectoryTree();
    void SetCurrentDirectory(const std::string& path);
    std::string GetCurrentDirectory() const { return m_CurrentDirectory; }
    
    // Set the project root (assets directory)
    void SetProjectRoot(const std::string& root);
    std::string GetProjectRoot() const { return m_AssetsRoot; }
    
    // Callback for scene opening
    void SetOnSceneOpened(std::function<void(const std::string&)> callback) { m_OnSceneOpened = callback; }
    
    // Callback for scene rename (called with old path, new path)
    void SetOnSceneRenamed(std::function<void(const std::string&, const std::string&)> callback) { m_OnSceneRenamed = callback; }

private:
    void RenderFolderTree();
    void RenderDirectoryNode(std::shared_ptr<DirectoryNode> node, int depth = 0);
    void RenderContentArea();
    void RenderAssetIcon(const FileEntry& entry);
    void RenderAssetThumbnail(const FileEntry& entry, float iconSize);
    
    void NavigateToDirectory(const std::string& path);
    void NavigateUp();
    
    // Directory tree building
    std::shared_ptr<DirectoryNode> BuildDirectoryTree(const std::string& rootPath);
    void ScanDirectory(std::shared_ptr<DirectoryNode> node);
    std::shared_ptr<DirectoryNode> FindNodeByPath(std::shared_ptr<DirectoryNode> root, const std::string& path);
    
    // File operations
    void OnFileSelected(const FileEntry& entry);
    void OnFileDoubleClicked(const FileEntry& entry);
    void OnDirectoryDoubleClicked(const std::string& path);
    void OnSceneNameEdited(const FileEntry& entry, const std::string& newName);
    
    std::string m_CurrentDirectory;
    std::string m_AssetsRoot;
    std::vector<FileEntry> m_CurrentDirectoryEntries;
    std::vector<std::string> m_DirectoryHistory;
    int m_HistoryIndex;
    
    // Directory tree
    std::shared_ptr<DirectoryNode> m_RootNode;
    std::set<std::string> m_ExpandedPaths;  // Track expanded folders
    
    // Asset filtering
    EFileType m_FilterType;
    bool m_ShowDirectories;
    bool m_ShowFiles;
    
    // View settings
    float m_ThumbnailSize;
    enum class ViewMode { Icons, List };
    ViewMode m_ViewMode;
    
    // Selected file
    std::string m_SelectedFilePath;
    
    // File editing state
    std::string m_EditingFilePath;  // Path of file being edited
    std::string m_EditingNameBuffer;  // Buffer for editing name
    
    // Callbacks
    std::function<void(const std::string&)> m_OnSceneOpened;
    std::function<void(const std::string&, const std::string&)> m_OnSceneRenamed;  // oldPath, newPath
    
    // Thumbnail textures
    std::shared_ptr<Texture> m_FolderThumbnail;
    std::shared_ptr<Texture> m_ScriptThumbnail;
    std::shared_ptr<Texture> m_ShaderThumbnail;
    
    void LoadThumbnails();
    
    // Helper to get display name (strips .lscene extension for scenes)
    std::string GetDisplayName(const FileEntry& entry) const;
};

} // namespace LGE


