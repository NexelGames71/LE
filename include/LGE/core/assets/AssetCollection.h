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
#include "LGE/core/assets/AssetFilter.h"
#include <string>
#include <vector>
#include <filesystem>
#include "imgui.h"

namespace LGE {

// Forward declarations
class AssetRegistry;
class AssetSearchIndex;

class AssetCollection {
public:
    enum class CollectionType {
        Manual,     // User manually adds assets
        Smart       // Auto-populated by query
    };
    
    std::string name;
    std::string description;
    CollectionType type;
    
    // For manual collections
    std::vector<GUID> assets;
    
    // For smart collections
    std::string query;
    AssetFilter::FilterCriteria criteria;
    
    // UI color
    ImVec4 color;
    
    AssetCollection()
        : type(CollectionType::Manual)
        , color(ImVec4(0.5f, 0.5f, 0.5f, 1.0f))
    {}
    
    std::string ToJson() const;
    static AssetCollection FromJson(const std::string& json);
};

class CollectionManager {
private:
    std::vector<AssetCollection> m_Collections;
    AssetRegistry* m_Registry;
    AssetSearchIndex* m_SearchIndex;
    std::filesystem::path m_StorageFile;

public:
    CollectionManager(AssetRegistry* reg, AssetSearchIndex* index);
    
    // Collection management
    AssetCollection* CreateCollection(const std::string& name, AssetCollection::CollectionType type);
    bool DeleteCollection(const std::string& name);
    AssetCollection* GetCollection(const std::string& name);
    const std::vector<AssetCollection>& GetAllCollections() const { return m_Collections; }
    
    // Manual collection operations
    bool AddToCollection(const std::string& collectionName, const GUID& asset);
    bool RemoveFromCollection(const std::string& collectionName, const GUID& asset);
    
    // Get assets in collection
    std::vector<GUID> GetCollectionAssets(const std::string& collectionName);
    
    // Update smart collections
    void UpdateSmartCollections();
    
    // Persistence
    void SetStorageFile(const std::filesystem::path& path) { m_StorageFile = path; }
    const std::filesystem::path& GetStorageFile() const { return m_StorageFile; }
    bool SaveToFile();
    bool LoadFromFile();
    
    void Clear() { m_Collections.clear(); }
    size_t GetCount() const { return m_Collections.size(); }
};

} // namespace LGE

