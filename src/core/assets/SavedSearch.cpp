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

#include "LGE/core/assets/SavedSearch.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <sstream>
#include <algorithm>

namespace LGE {

std::string SavedSearch::ToJson() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"name\": \"" << name << "\",\n";
    json << "  \"query\": \"" << query << "\",\n";
    json << "  \"types\": [\n";
    for (size_t i = 0; i < criteria.types.size(); ++i) {
        json << "    " << static_cast<int>(criteria.types[i]);
        if (i < criteria.types.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ],\n";
    json << "  \"tags\": [\n";
    for (size_t i = 0; i < criteria.tags.size(); ++i) {
        json << "    \"" << criteria.tags[i] << "\"";
        if (i < criteria.tags.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ],\n";
    json << "  \"namePattern\": \"" << criteria.namePattern << "\"\n";
    json << "}\n";
    return json.str();
}

SavedSearch SavedSearch::FromJson(const std::string& json) {
    SavedSearch search;
    
    // Parse name
    size_t namePos = json.find("\"name\"");
    if (namePos != std::string::npos) {
        size_t colonPos = json.find(':', namePos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                search.name = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Parse query
    size_t queryPos = json.find("\"query\"");
    if (queryPos != std::string::npos) {
        size_t colonPos = json.find(':', queryPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                search.query = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Parse types array
    size_t typesPos = json.find("\"types\"");
    if (typesPos != std::string::npos) {
        size_t arrayStart = json.find('[', typesPos);
        size_t arrayEnd = json.find(']', arrayStart);
        if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
            std::string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            size_t pos = 0;
            while (pos < arrayContent.length()) {
                // Skip whitespace
                while (pos < arrayContent.length() && std::isspace(arrayContent[pos])) {
                    pos++;
                }
                if (pos >= arrayContent.length()) break;
                
                // Find number
                size_t numStart = pos;
                while (pos < arrayContent.length() && std::isdigit(arrayContent[pos])) {
                    pos++;
                }
                if (pos > numStart) {
                    std::string numStr = arrayContent.substr(numStart, pos - numStart);
                    int typeValue = std::stoi(numStr);
                    search.criteria.types.push_back(static_cast<AssetType>(typeValue));
                }
                
                // Skip comma
                if (pos < arrayContent.length() && arrayContent[pos] == ',') {
                    pos++;
                }
            }
        }
    }
    
    // Parse tags array
    size_t tagsPos = json.find("\"tags\"");
    if (tagsPos != std::string::npos) {
        size_t arrayStart = json.find('[', tagsPos);
        size_t arrayEnd = json.find(']', arrayStart);
        if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
            std::string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            size_t pos = 0;
            while ((pos = arrayContent.find('"', pos)) != std::string::npos) {
                size_t tagStart = pos;
                size_t tagEnd = arrayContent.find('"', tagStart + 1);
                if (tagEnd != std::string::npos) {
                    std::string tag = arrayContent.substr(tagStart + 1, tagEnd - tagStart - 1);
                    search.criteria.tags.push_back(tag);
                    pos = tagEnd + 1;
                } else {
                    break;
                }
            }
        }
    }
    
    // Parse namePattern
    size_t patternPos = json.find("\"namePattern\"");
    if (patternPos != std::string::npos) {
        size_t colonPos = json.find(':', patternPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                search.criteria.namePattern = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    return search;
}

SavedSearchManager::SavedSearchManager()
    : m_StorageFile("saved_searches.json")
{
}

SavedSearchManager::SavedSearchManager(const std::filesystem::path& storageFile)
    : m_StorageFile(storageFile)
{
}

void SavedSearchManager::AddSearch(const SavedSearch& search) {
    // Remove existing search with same name
    RemoveSearch(search.name);
    m_SavedSearches.push_back(search);
}

void SavedSearchManager::RemoveSearch(const std::string& name) {
    m_SavedSearches.erase(
        std::remove_if(m_SavedSearches.begin(), m_SavedSearches.end(),
            [&name](const SavedSearch& s) { return s.name == name; }),
        m_SavedSearches.end()
    );
}

SavedSearch* SavedSearchManager::GetSearch(const std::string& name) {
    auto it = std::find_if(m_SavedSearches.begin(), m_SavedSearches.end(),
        [&name](const SavedSearch& s) { return s.name == name; });
    if (it != m_SavedSearches.end()) {
        return &(*it);
    }
    return nullptr;
}

bool SavedSearchManager::SaveToFile() {
    if (m_StorageFile.empty()) {
        Log::Error("Storage file path not set for SavedSearchManager");
        return false;
    }
    
    std::ostringstream json;
    json << "{\n";
    json << "  \"searches\": [\n";
    
    for (size_t i = 0; i < m_SavedSearches.size(); ++i) {
        std::string searchJson = m_SavedSearches[i].ToJson();
        // Indent the search JSON
        std::istringstream iss(searchJson);
        std::string line;
        while (std::getline(iss, line)) {
            json << "    " << line << "\n";
        }
        if (i < m_SavedSearches.size() - 1) {
            json << "    ,\n";
        }
    }
    
    json << "  ]\n";
    json << "}\n";
    
    if (!FileSystem::WriteFile(m_StorageFile.string(), json.str())) {
        Log::Error("Failed to save saved searches to: " + m_StorageFile.string());
        return false;
    }
    
    Log::Info("Saved searches saved to: " + m_StorageFile.string());
    return true;
}

bool SavedSearchManager::LoadFromFile() {
    if (m_StorageFile.empty()) {
        Log::Warn("Storage file path not set for SavedSearchManager");
        return false;
    }
    
    if (!FileSystem::Exists(m_StorageFile.string())) {
        Log::Info("Saved searches file not found: " + m_StorageFile.string());
        return true; // Not an error
    }
    
    std::string jsonString = FileSystem::ReadFile(m_StorageFile.string());
    if (jsonString.empty()) {
        Log::Error("Failed to read saved searches from: " + m_StorageFile.string());
        return false;
    }
    
    Clear();
    
    // Find searches array
    size_t searchesPos = jsonString.find("\"searches\"");
    if (searchesPos == std::string::npos) {
        Log::Warn("No searches array found in saved searches file");
        return true;
    }
    
    size_t arrayStart = jsonString.find('[', searchesPos);
    if (arrayStart == std::string::npos) {
        return false;
    }
    
    // Parse each search object
    size_t pos = arrayStart + 1;
    int braceDepth = 0;
    size_t objStart = 0;
    
    while (pos < jsonString.length()) {
        if (jsonString[pos] == '{') {
            if (braceDepth == 0) {
                objStart = pos;
            }
            braceDepth++;
        } else if (jsonString[pos] == '}') {
            braceDepth--;
            if (braceDepth == 0) {
                // Extract complete JSON object
                std::string searchJson = jsonString.substr(objStart, pos - objStart + 1);
                SavedSearch search = SavedSearch::FromJson(searchJson);
                if (!search.name.empty()) {
                    m_SavedSearches.push_back(search);
                }
            }
        }
        pos++;
    }
    
    Log::Info("Loaded " + std::to_string(m_SavedSearches.size()) + " saved searches from: " + m_StorageFile.string());
    return true;
}

} // namespace LGE

