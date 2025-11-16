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

#include "LGE/core/assets/AssetFilter.h"
#include <string>
#include <filesystem>

namespace LGE {

class SavedSearch {
public:
    std::string name;
    std::string query;
    AssetFilter::FilterCriteria criteria;
    
    SavedSearch() {}
    SavedSearch(const std::string& n, const std::string& q, const AssetFilter::FilterCriteria& c)
        : name(n), query(q), criteria(c) {}
    
    std::string ToJson() const;
    static SavedSearch FromJson(const std::string& json);
};

class SavedSearchManager {
private:
    std::vector<SavedSearch> m_SavedSearches;
    std::filesystem::path m_StorageFile;

public:
    SavedSearchManager();
    explicit SavedSearchManager(const std::filesystem::path& storageFile);
    
    void AddSearch(const SavedSearch& search);
    void RemoveSearch(const std::string& name);
    SavedSearch* GetSearch(const std::string& name);
    const std::vector<SavedSearch>& GetAllSearches() const { return m_SavedSearches; }
    
    void SetStorageFile(const std::filesystem::path& path) { m_StorageFile = path; }
    const std::filesystem::path& GetStorageFile() const { return m_StorageFile; }
    
    bool SaveToFile();
    bool LoadFromFile();
    
    void Clear() { m_SavedSearches.clear(); }
    size_t GetCount() const { return m_SavedSearches.size(); }
};

} // namespace LGE

