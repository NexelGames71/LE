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

#include "LGE/core/assets/AssetSearchIndex.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/Log.h"
#include <algorithm>
#include <sstream>
#include <cctype>

namespace LGE {

void AssetSearchIndex::RebuildIndex(AssetRegistry* registry) {
    if (!registry) {
        Log::Error("AssetRegistry is null in AssetSearchIndex::RebuildIndex");
        return;
    }
    
    Clear();
    
    // Get all assets
    std::vector<AssetType> allTypes = {
        AssetType::Texture, AssetType::Model, AssetType::Material,
        AssetType::Shader, AssetType::Script, AssetType::Audio,
        AssetType::Scene, AssetType::Prefab, AssetType::Animation, AssetType::Font
    };
    
    for (AssetType type : allTypes) {
        std::vector<AssetMetadata*> assets = registry->GetAssetsByType(type);
        for (AssetMetadata* metadata : assets) {
            if (metadata && metadata->guid.IsValid()) {
                UpdateAsset(metadata->guid, registry);
            }
        }
    }
    
    BuildWordIndex();
    Log::Info("Asset search index rebuilt with " + std::to_string(m_Entries.size()) + " entries");
}

void AssetSearchIndex::UpdateAsset(const GUID& guid, AssetRegistry* registry) {
    if (!registry) {
        return;
    }
    
    const AssetMetadata* metadata = registry->GetAsset(guid);
    if (!metadata) {
        RemoveAsset(guid);
        return;
    }
    
    // Remove old entry if exists
    auto it = m_GuidToIndex.find(guid);
    if (it != m_GuidToIndex.end()) {
        size_t oldIndex = it->second;
        // Remove from word index
        std::vector<std::string> oldWords = Tokenize(m_Entries[oldIndex].searchableText);
        for (const auto& word : oldWords) {
            auto wordIt = m_WordIndex.find(word);
            if (wordIt != m_WordIndex.end()) {
                auto& guids = wordIt->second;
                guids.erase(std::remove(guids.begin(), guids.end(), guid), guids.end());
                if (guids.empty()) {
                    m_WordIndex.erase(wordIt);
                }
            }
        }
        // Remove entry
        m_Entries.erase(m_Entries.begin() + oldIndex);
        // Update indices
        m_GuidToIndex.clear();
        for (size_t i = 0; i < m_Entries.size(); ++i) {
            m_GuidToIndex[m_Entries[i].guid] = i;
        }
    }
    
    // Create new entry
    SearchEntry entry;
    entry.guid = metadata->guid;
    entry.name = metadata->name;
    entry.path = metadata->virtualPath;
    entry.type = metadata->type;
    entry.tags = metadata->tags;
    
    // Build searchable text
    std::ostringstream searchText;
    searchText << metadata->name << " ";
    searchText << metadata->virtualPath << " ";
    for (const auto& tag : metadata->tags) {
        searchText << tag << " ";
    }
    entry.searchableText = searchText.str();
    
    // Add to entries
    size_t index = m_Entries.size();
    m_Entries.push_back(entry);
    m_GuidToIndex[guid] = index;
    
    // Add to word index
    std::vector<std::string> words = Tokenize(entry.searchableText);
    for (const auto& word : words) {
        m_WordIndex[word].push_back(guid);
    }
}

void AssetSearchIndex::RemoveAsset(const GUID& guid) {
    auto it = m_GuidToIndex.find(guid);
    if (it == m_GuidToIndex.end()) {
        return;
    }
    
    size_t index = it->second;
    SearchEntry& entry = m_Entries[index];
    
    // Remove from word index
    std::vector<std::string> words = Tokenize(entry.searchableText);
    for (const auto& word : words) {
        auto wordIt = m_WordIndex.find(word);
        if (wordIt != m_WordIndex.end()) {
            auto& guids = wordIt->second;
            guids.erase(std::remove(guids.begin(), guids.end(), guid), guids.end());
            if (guids.empty()) {
                m_WordIndex.erase(wordIt);
            }
        }
    }
    
    // Remove entry
    m_Entries.erase(m_Entries.begin() + index);
    
    // Rebuild index map
    m_GuidToIndex.clear();
    for (size_t i = 0; i < m_Entries.size(); ++i) {
        m_GuidToIndex[m_Entries[i].guid] = i;
    }
}

std::vector<GUID> AssetSearchIndex::Search(const std::string& query) {
    if (query.empty()) {
        return {};
    }
    
    std::string lowerQuery = ToLower(query);
    std::vector<std::string> queryWords = Tokenize(lowerQuery);
    
    if (queryWords.empty()) {
        return {};
    }
    
    // Find assets that contain all query words (AND search)
    std::unordered_map<GUID, int> assetScores;
    
    for (const auto& word : queryWords) {
        auto wordIt = m_WordIndex.find(word);
        if (wordIt != m_WordIndex.end()) {
            for (const auto& guid : wordIt->second) {
                assetScores[guid]++;
            }
        }
    }
    
    // Filter to only assets that match all words
    std::vector<GUID> results;
    int requiredMatches = static_cast<int>(queryWords.size());
    
    for (const auto& pair : assetScores) {
        if (pair.second == requiredMatches) {
            results.push_back(pair.first);
        }
    }
    
    return results;
}

std::vector<std::string> AssetSearchIndex::Tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string lowerText = ToLower(text);
    
    std::istringstream iss(lowerText);
    std::string token;
    
    while (iss >> token) {
        // Remove punctuation
        token.erase(std::remove_if(token.begin(), token.end(), 
            [](char c) { return !std::isalnum(c); }), token.end());
        
        if (!token.empty() && token.length() >= 2) { // Ignore single characters
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

std::string AssetSearchIndex::ToLower(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

void AssetSearchIndex::BuildWordIndex() {
    m_WordIndex.clear();
    
    for (const auto& entry : m_Entries) {
        std::vector<std::string> words = Tokenize(entry.searchableText);
        for (const auto& word : words) {
            m_WordIndex[word].push_back(entry.guid);
        }
    }
}

void AssetSearchIndex::Clear() {
    m_Entries.clear();
    m_GuidToIndex.clear();
    m_WordIndex.clear();
}

} // namespace LGE

