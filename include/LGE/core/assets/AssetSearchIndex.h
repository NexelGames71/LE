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
#include "LGE/core/assets/AssetMetadata.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace LGE {

// Forward declaration
class AssetRegistry;

class AssetSearchIndex {
private:
    struct SearchEntry {
        GUID guid;
        std::string name;
        std::string path;
        AssetType type;
        std::vector<std::string> tags;
        std::string searchableText; // Combined text for full-text search
        
        SearchEntry() : type(AssetType::Unknown) {}
    };
    
    std::vector<SearchEntry> m_Entries;
    std::unordered_map<GUID, size_t> m_GuidToIndex;
    
    // Inverted index for fast text search
    std::unordered_map<std::string, std::vector<GUID>> m_WordIndex;
    
    void BuildWordIndex();
    std::vector<std::string> Tokenize(const std::string& text);
    std::string ToLower(const std::string& str) const;

public:
    void RebuildIndex(AssetRegistry* registry);
    void UpdateAsset(const GUID& guid, AssetRegistry* registry);
    void RemoveAsset(const GUID& guid);
    
    std::vector<GUID> Search(const std::string& query);
    
    // Get index statistics
    size_t GetEntryCount() const { return m_Entries.size(); }
    size_t GetWordCount() const { return m_WordIndex.size(); }
    
    // Clear index
    void Clear();
};

} // namespace LGE

