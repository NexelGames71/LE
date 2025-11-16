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

#include "LGE/ui/ContentBrowserModel.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <algorithm>
#include <cctype>

namespace LGE {

ContentBrowserModel::ContentBrowserModel(AssetRegistry* reg, VirtualFileSystem* vfs)
    : m_Registry(reg)
    , m_VFS(vfs)
{
    NavigateToRoot();
}

void ContentBrowserModel::NavigateToDirectory(const std::string& path) {
    std::filesystem::path dirPath = m_VFS ? m_VFS->ResolveVirtualPath(path) : path;
    
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        Log::Warn("Directory does not exist: " + path);
        return;
    }
    
    m_CurrentDirectory = dirPath;
    RefreshCurrentDirectory();
    
    if (onItemsChanged) {
        onItemsChanged();
    }
}

void ContentBrowserModel::NavigateUp() {
    if (m_CurrentDirectory.has_parent_path()) {
        m_CurrentDirectory = m_CurrentDirectory.parent_path();
        RefreshCurrentDirectory();
        
        if (onItemsChanged) {
            onItemsChanged();
        }
    }
}

void ContentBrowserModel::NavigateToRoot() {
    if (m_VFS) {
        std::filesystem::path assetsPath = m_VFS->ResolveVirtualPath("/Assets");
        if (std::filesystem::exists(assetsPath)) {
            m_CurrentDirectory = assetsPath;
        } else {
            m_CurrentDirectory = std::filesystem::current_path() / "Assets";
        }
    } else {
        m_CurrentDirectory = std::filesystem::current_path() / "Assets";
    }
    
    RefreshCurrentDirectory();
    
    if (onItemsChanged) {
        onItemsChanged();
    }
}

std::string ContentBrowserModel::GetCurrentPath() const {
    if (m_VFS) {
        return m_VFS->GetVirtualPath(m_CurrentDirectory);
    }
    return m_CurrentDirectory.string();
}

void ContentBrowserModel::RefreshCurrentDirectory() {
    m_CurrentItems.clear();
    
    if (!std::filesystem::exists(m_CurrentDirectory)) {
        return;
    }
    
    // Add folders first
    try {
        for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
            if (entry.is_directory()) {
                ContentBrowserItem item;
                item.type = ContentBrowserItem::ItemType::Folder;
                item.name = entry.path().filename().string();
                item.path = m_VFS ? m_VFS->GetVirtualPath(entry.path()) : entry.path().string();
                m_CurrentItems.push_back(item);
            }
        }
    } catch (const std::exception& e) {
        Log::Error("Error reading directory: " + std::string(e.what()));
    }
    
    // Add assets
    if (m_Registry) {
        // Get all assets and filter by current directory
        std::string currentVirtualPath = GetCurrentPath();
        
        // Get all assets by iterating through types
        std::vector<AssetType> allTypes = {
            AssetType::Texture, AssetType::Model, AssetType::Material,
            AssetType::Shader, AssetType::Script, AssetType::Audio,
            AssetType::Scene, AssetType::Prefab, AssetType::Animation, AssetType::Font
        };
        
        for (AssetType type : allTypes) {
            std::vector<AssetMetadata*> assets = m_Registry->GetAssetsByType(type);
            for (AssetMetadata* metadataPtr : assets) {
                const AssetMetadata& metadata = *metadataPtr;
                std::string assetDir = std::filesystem::path(metadata.virtualPath).parent_path().string();
                std::replace(assetDir.begin(), assetDir.end(), '\\', '/');
                
                std::string currentDir = currentVirtualPath;
                std::replace(currentDir.begin(), currentDir.end(), '\\', '/');
                
                if (assetDir == currentDir || assetDir + "/" == currentDir || assetDir == currentDir + "/") {
                    // Apply search filter
                    if (!m_SearchQuery.empty()) {
                        std::string lowerName = metadata.name;
                        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                        std::string lowerQuery = m_SearchQuery;
                        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
                        
                        if (lowerName.find(lowerQuery) == std::string::npos) {
                            continue;
                        }
                    }
                    
                    // Apply type filter
                    if (!m_TypeFilters.empty()) {
                        if (std::find(m_TypeFilters.begin(), m_TypeFilters.end(), metadata.type) == m_TypeFilters.end()) {
                            continue;
                        }
                    }
                    
                    ContentBrowserItem item;
                    item.type = ContentBrowserItem::ItemType::Asset;
                    item.name = metadata.name;
                    item.path = metadata.virtualPath;
                    item.guid = metadata.guid;
                    item.assetType = metadata.type;
                    m_CurrentItems.push_back(item);
                }
            }
        }
    }
    
    // Sort: folders first, then by name
    std::sort(m_CurrentItems.begin(), m_CurrentItems.end(),
        [](const ContentBrowserItem& a, const ContentBrowserItem& b) {
            if (a.type != b.type) {
                return a.type == ContentBrowserItem::ItemType::Folder;
            }
            return a.name < b.name;
        });
}

void ContentBrowserModel::BuildFolderHierarchy() {
    // This would build a tree structure for the folder view
    // For now, we'll keep it simple with flat directory listing
}

void ContentBrowserModel::SelectItem(const ContentBrowserItem& item, bool addToSelection) {
    if (!addToSelection) {
        m_SelectedItems.clear();
    }
    
    // Check if already selected
    auto it = std::find_if(m_SelectedItems.begin(), m_SelectedItems.end(),
        [&item](const ContentBrowserItem& selected) {
            return selected.path == item.path;
        });
    
    if (it == m_SelectedItems.end()) {
        m_SelectedItems.push_back(item);
    }
    
    if (onSelectionChanged) {
        onSelectionChanged();
    }
}

void ContentBrowserModel::ClearSelection() {
    m_SelectedItems.clear();
    if (onSelectionChanged) {
        onSelectionChanged();
    }
}

void ContentBrowserModel::SetSearchQuery(const std::string& query) {
    m_SearchQuery = query;
    RefreshCurrentDirectory();
    if (onItemsChanged) {
        onItemsChanged();
    }
}

void ContentBrowserModel::SetTypeFilter(const std::vector<AssetType>& types) {
    m_TypeFilters = types;
    RefreshCurrentDirectory();
    if (onItemsChanged) {
        onItemsChanged();
    }
}

void ContentBrowserModel::ClearFilters() {
    m_SearchQuery.clear();
    m_TypeFilters.clear();
    RefreshCurrentDirectory();
    if (onItemsChanged) {
        onItemsChanged();
    }
}

bool ContentBrowserModel::CreateFolder(const std::string& name) {
    std::filesystem::path newFolder = m_CurrentDirectory / name;
    
    if (std::filesystem::exists(newFolder)) {
        Log::Warn("Folder already exists: " + name);
        return false;
    }
    
    if (std::filesystem::create_directory(newFolder)) {
        RefreshCurrentDirectory();
        if (onItemsChanged) {
            onItemsChanged();
        }
        return true;
    }
    
    return false;
}

bool ContentBrowserModel::DeleteSelected() {
    bool success = true;
    
    for (const auto& item : m_SelectedItems) {
        std::filesystem::path itemPath = m_VFS ? m_VFS->ResolveVirtualPath(item.path) : item.path;
        
        try {
            if (item.type == ContentBrowserItem::ItemType::Folder) {
                std::filesystem::remove_all(itemPath);
            } else {
                std::filesystem::remove(itemPath);
                // Also remove from registry
                if (m_Registry) {
                    m_Registry->UnregisterAsset(item.guid);
                }
            }
        } catch (const std::exception& e) {
            Log::Error("Failed to delete: " + item.path + " - " + std::string(e.what()));
            success = false;
        }
    }
    
    if (success) {
        m_SelectedItems.clear();
        RefreshCurrentDirectory();
        if (onItemsChanged) {
            onItemsChanged();
        }
    }
    
    return success;
}

bool ContentBrowserModel::RenameItem(const ContentBrowserItem& item, const std::string& newName) {
    std::filesystem::path oldPath = m_VFS ? m_VFS->ResolveVirtualPath(item.path) : item.path;
    std::filesystem::path newPath = oldPath.parent_path() / newName;
    
    if (std::filesystem::exists(newPath)) {
        Log::Warn("Name already exists: " + newName);
        return false;
    }
    
    try {
        std::filesystem::rename(oldPath, newPath);
        
        // Update registry if it's an asset
        if (item.type == ContentBrowserItem::ItemType::Asset && m_Registry) {
            AssetMetadata* metadata = m_Registry->GetAsset(item.guid);
            if (metadata) {
                std::string newVirtualPath = m_VFS ? m_VFS->GetVirtualPath(newPath) : newPath.string();
                metadata->virtualPath = newVirtualPath;
                metadata->name = newName;
                m_Registry->UpdateAsset(item.guid, *metadata);
            }
        }
        
        RefreshCurrentDirectory();
        if (onItemsChanged) {
            onItemsChanged();
        }
        return true;
    } catch (const std::exception& e) {
        Log::Error("Failed to rename: " + std::string(e.what()));
        return false;
    }
}

bool ContentBrowserModel::DuplicateItem(const ContentBrowserItem& item) {
    std::filesystem::path sourcePath = m_VFS ? m_VFS->ResolveVirtualPath(item.path) : item.path;
    std::filesystem::path destPath = sourcePath.parent_path() / (sourcePath.stem().string() + "_Copy" + sourcePath.extension().string());
    
    try {
        if (item.type == ContentBrowserItem::ItemType::Folder) {
            std::filesystem::copy(sourcePath, destPath, std::filesystem::copy_options::recursive);
        } else {
            std::filesystem::copy_file(sourcePath, destPath);
            // Register new asset in registry
            if (m_Registry && item.type == ContentBrowserItem::ItemType::Asset) {
                AssetMetadata* original = m_Registry->GetAsset(item.guid);
                if (original) {
                    AssetMetadata duplicate = *original;
                    duplicate.guid = GUID::Generate();
                    duplicate.name = destPath.stem().string();
                    duplicate.virtualPath = m_VFS ? m_VFS->GetVirtualPath(destPath) : destPath.string();
                    m_Registry->RegisterAsset(duplicate);
                }
            }
        }
        
        RefreshCurrentDirectory();
        if (onItemsChanged) {
            onItemsChanged();
        }
        return true;
    } catch (const std::exception& e) {
        Log::Error("Failed to duplicate: " + std::string(e.what()));
        return false;
    }
}

bool ContentBrowserModel::CanDrop(const std::vector<std::filesystem::path>& paths) {
    // Check if any paths can be imported
    return !paths.empty();
}

bool ContentBrowserModel::HandleDrop(const std::vector<std::filesystem::path>& paths) {
    // This would trigger the import process
    // For now, just log
    for (const auto& path : paths) {
        Log::Info("Dropped file: " + path.string());
    }
    return true;
}

} // namespace LGE

