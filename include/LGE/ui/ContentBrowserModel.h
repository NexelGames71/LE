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

#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/GUID.h"
#include "LGE/core/filesystem/VirtualFileSystem.h"
#include "LGE/core/assets/AssetMetadata.h"
#include "LGE/rendering/Texture.h"
#include <filesystem>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace LGE {

struct ContentBrowserItem {
    enum class ItemType {
        Folder,
        Asset
    };
    
    ItemType type;
    std::string name;
    std::string path;
    GUID guid;  // Only for assets
    AssetType assetType;  // Only for assets
    std::shared_ptr<Texture> thumbnail;
    
    // For folders
    std::vector<ContentBrowserItem> children;
    
    ContentBrowserItem()
        : type(ItemType::Folder)
        , guid(GUID::Invalid())
        , assetType(AssetType::Unknown)
    {}
};

class ContentBrowserModel {
private:
    AssetRegistry* m_Registry;
    VirtualFileSystem* m_VFS;
    std::filesystem::path m_CurrentDirectory;
    std::vector<ContentBrowserItem> m_CurrentItems;
    std::vector<ContentBrowserItem> m_SelectedItems;
    
    // Search and filter
    std::string m_SearchQuery;
    std::vector<AssetType> m_TypeFilters;
    
    void RefreshCurrentDirectory();
    void BuildFolderHierarchy();

public:
    ContentBrowserModel(AssetRegistry* reg, VirtualFileSystem* vfs);
    
    // Navigation
    void NavigateToDirectory(const std::string& path);
    void NavigateUp();
    void NavigateToRoot();
    std::string GetCurrentPath() const;
    
    // Get items
    const std::vector<ContentBrowserItem>& GetCurrentItems() const { return m_CurrentItems; }
    const std::vector<ContentBrowserItem>& GetSelectedItems() const { return m_SelectedItems; }
    
    // Selection
    void SelectItem(const ContentBrowserItem& item, bool addToSelection = false);
    void ClearSelection();
    
    // Search and filter
    void SetSearchQuery(const std::string& query);
    void SetTypeFilter(const std::vector<AssetType>& types);
    void ClearFilters();
    
    // Operations
    bool CreateFolder(const std::string& name);
    bool DeleteSelected();
    bool RenameItem(const ContentBrowserItem& item, const std::string& newName);
    bool DuplicateItem(const ContentBrowserItem& item);
    
    // Drag and drop
    bool CanDrop(const std::vector<std::filesystem::path>& paths);
    bool HandleDrop(const std::vector<std::filesystem::path>& paths);
    
    // Callbacks for UI updates
    std::function<void()> onItemsChanged;
    std::function<void()> onSelectionChanged;
    
    // Set VFS
    void SetVFS(VirtualFileSystem* vfs) { m_VFS = vfs; }
};

} // namespace LGE

