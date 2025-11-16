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

#include "LGE/core/assets/AssetCollection.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/assets/AssetSearchIndex.h"
#include "LGE/core/assets/AssetFilter.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace LGE {

std::string AssetCollection::ToJson() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"name\": \"" << name << "\",\n";
    json << "  \"description\": \"" << description << "\",\n";
    json << "  \"type\": " << static_cast<int>(type) << ",\n";
    json << "  \"color\": [" << color.x << ", " << color.y << ", " << color.z << ", " << color.w << "],\n";
    
    if (type == CollectionType::Manual) {
        json << "  \"assets\": [\n";
        for (size_t i = 0; i < assets.size(); ++i) {
            json << "    \"" << assets[i].ToString() << "\"";
            if (i < assets.size() - 1) {
                json << ",";
            }
            json << "\n";
        }
        json << "  ]\n";
    } else {
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
        json << "  ]\n";
    }
    
    json << "}\n";
    return json.str();
}

AssetCollection AssetCollection::FromJson(const std::string& json) {
    AssetCollection collection;
    
    // Parse name
    size_t namePos = json.find("\"name\"");
    if (namePos != std::string::npos) {
        size_t colonPos = json.find(':', namePos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                collection.name = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Parse description
    size_t descPos = json.find("\"description\"");
    if (descPos != std::string::npos) {
        size_t colonPos = json.find(':', descPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                collection.description = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Parse type
    size_t typePos = json.find("\"type\"");
    if (typePos != std::string::npos) {
        size_t colonPos = json.find(':', typePos);
        size_t numStart = json.find_first_of("0123456789", colonPos);
        if (numStart != std::string::npos) {
            size_t numEnd = numStart;
            while (numEnd < json.length() && std::isdigit(json[numEnd])) {
                numEnd++;
            }
            int typeValue = std::stoi(json.substr(numStart, numEnd - numStart));
            collection.type = static_cast<CollectionType>(typeValue);
        }
    }
    
    // Parse color
    size_t colorPos = json.find("\"color\"");
    if (colorPos != std::string::npos) {
        size_t arrayStart = json.find('[', colorPos);
        size_t arrayEnd = json.find(']', arrayStart);
        if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
            std::string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            std::istringstream iss(arrayContent);
            float r, g, b, a;
            char comma;
            if (iss >> r >> comma >> g >> comma >> b >> comma >> a) {
                collection.color = ImVec4(r, g, b, a);
            }
        }
    }
    
    if (collection.type == CollectionType::Manual) {
        // Parse assets array
        size_t assetsPos = json.find("\"assets\"");
        if (assetsPos != std::string::npos) {
            size_t arrayStart = json.find('[', assetsPos);
            size_t arrayEnd = json.find(']', arrayStart);
            if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
                std::string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                size_t pos = 0;
                while ((pos = arrayContent.find('"', pos)) != std::string::npos) {
                    size_t guidStart = pos;
                    size_t guidEnd = arrayContent.find('"', guidStart + 1);
                    if (guidEnd != std::string::npos) {
                        std::string guidStr = arrayContent.substr(guidStart + 1, guidEnd - guidStart - 1);
                        GUID guid = GUID::FromString(guidStr);
                        if (guid.IsValid()) {
                            collection.assets.push_back(guid);
                        }
                        pos = guidEnd + 1;
                    } else {
                        break;
                    }
                }
            }
        }
    } else {
        // Parse query
        size_t queryPos = json.find("\"query\"");
        if (queryPos != std::string::npos) {
            size_t colonPos = json.find(':', queryPos);
            size_t quoteStart = json.find('"', colonPos);
            if (quoteStart != std::string::npos) {
                size_t quoteEnd = json.find('"', quoteStart + 1);
                if (quoteEnd != std::string::npos) {
                    collection.query = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                }
            }
        }
        
        // Parse criteria (similar to SavedSearch)
        size_t typesPos = json.find("\"types\"");
        if (typesPos != std::string::npos) {
            size_t arrayStart = json.find('[', typesPos);
            size_t arrayEnd = json.find(']', arrayStart);
            if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
                std::string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                size_t pos = 0;
                while (pos < arrayContent.length()) {
                    while (pos < arrayContent.length() && std::isspace(arrayContent[pos])) {
                        pos++;
                    }
                    if (pos >= arrayContent.length()) break;
                    
                    size_t numStart = pos;
                    while (pos < arrayContent.length() && std::isdigit(arrayContent[pos])) {
                        pos++;
                    }
                    if (pos > numStart) {
                        std::string numStr = arrayContent.substr(numStart, pos - numStart);
                        int typeValue = std::stoi(numStr);
                        collection.criteria.types.push_back(static_cast<AssetType>(typeValue));
                    }
                    
                    if (pos < arrayContent.length() && arrayContent[pos] == ',') {
                        pos++;
                    }
                }
            }
        }
    }
    
    return collection;
}

CollectionManager::CollectionManager(AssetRegistry* reg, AssetSearchIndex* index)
    : m_Registry(reg)
    , m_SearchIndex(index)
    , m_StorageFile("collections.json")
{
}

AssetCollection* CollectionManager::CreateCollection(const std::string& name, AssetCollection::CollectionType type) {
    // Check if collection with same name exists
    if (GetCollection(name) != nullptr) {
        Log::Warn("Collection with name already exists: " + name);
        return nullptr;
    }
    
    AssetCollection collection;
    collection.name = name;
    collection.type = type;
    m_Collections.push_back(collection);
    
    Log::Info("Created collection: " + name);
    return &m_Collections.back();
}

bool CollectionManager::DeleteCollection(const std::string& name) {
    auto it = std::find_if(m_Collections.begin(), m_Collections.end(),
        [&name](const AssetCollection& c) { return c.name == name; });
    if (it != m_Collections.end()) {
        m_Collections.erase(it);
        Log::Info("Deleted collection: " + name);
        return true;
    }
    return false;
}

AssetCollection* CollectionManager::GetCollection(const std::string& name) {
    auto it = std::find_if(m_Collections.begin(), m_Collections.end(),
        [&name](const AssetCollection& c) { return c.name == name; });
    if (it != m_Collections.end()) {
        return &(*it);
    }
    return nullptr;
}

bool CollectionManager::AddToCollection(const std::string& collectionName, const GUID& asset) {
    AssetCollection* collection = GetCollection(collectionName);
    if (!collection) {
        return false;
    }
    
    if (collection->type != AssetCollection::CollectionType::Manual) {
        Log::Warn("Cannot manually add assets to smart collection: " + collectionName);
        return false;
    }
    
    // Check if already in collection
    if (std::find(collection->assets.begin(), collection->assets.end(), asset) != collection->assets.end()) {
        return true; // Already added
    }
    
    collection->assets.push_back(asset);
    Log::Info("Added asset to collection: " + collectionName);
    return true;
}

bool CollectionManager::RemoveFromCollection(const std::string& collectionName, const GUID& asset) {
    AssetCollection* collection = GetCollection(collectionName);
    if (!collection) {
        return false;
    }
    
    if (collection->type != AssetCollection::CollectionType::Manual) {
        Log::Warn("Cannot manually remove assets from smart collection: " + collectionName);
        return false;
    }
    
    auto it = std::find(collection->assets.begin(), collection->assets.end(), asset);
    if (it != collection->assets.end()) {
        collection->assets.erase(it);
        Log::Info("Removed asset from collection: " + collectionName);
        return true;
    }
    return false;
}

std::vector<GUID> CollectionManager::GetCollectionAssets(const std::string& collectionName) {
    AssetCollection* collection = GetCollection(collectionName);
    if (!collection) {
        return {};
    }
    
    if (collection->type == AssetCollection::CollectionType::Manual) {
        return collection->assets;
    } else {
        // Smart collection - evaluate query
        std::vector<GUID> allAssets;
        if (m_SearchIndex && !collection->query.empty()) {
            allAssets = m_SearchIndex->Search(collection->query);
        } else if (m_Registry) {
            // Get all assets and filter
            std::vector<AssetType> allTypes = {
                AssetType::Texture, AssetType::Model, AssetType::Material,
                AssetType::Shader, AssetType::Script, AssetType::Audio,
                AssetType::Scene, AssetType::Prefab, AssetType::Animation, AssetType::Font
            };
            for (AssetType type : allTypes) {
                std::vector<AssetMetadata*> assets = m_Registry->GetAssetsByType(type);
                for (AssetMetadata* metadata : assets) {
                    if (metadata) {
                        allAssets.push_back(metadata->guid);
                    }
                }
            }
        }
        
        return AssetFilter::ApplyFilter(allAssets, collection->criteria, m_Registry);
    }
}

void CollectionManager::UpdateSmartCollections() {
    // Smart collections are updated on-demand when GetCollectionAssets is called
    // This method could trigger a refresh if needed
    Log::Info("Smart collections updated");
}

bool CollectionManager::SaveToFile() {
    if (m_StorageFile.empty()) {
        Log::Error("Storage file path not set for CollectionManager");
        return false;
    }
    
    std::ostringstream json;
    json << "{\n";
    json << "  \"collections\": [\n";
    
    for (size_t i = 0; i < m_Collections.size(); ++i) {
        std::string collectionJson = m_Collections[i].ToJson();
        // Indent the collection JSON
        std::istringstream iss(collectionJson);
        std::string line;
        while (std::getline(iss, line)) {
            json << "    " << line << "\n";
        }
        if (i < m_Collections.size() - 1) {
            json << "    ,\n";
        }
    }
    
    json << "  ]\n";
    json << "}\n";
    
    if (!FileSystem::WriteFile(m_StorageFile.string(), json.str())) {
        Log::Error("Failed to save collections to: " + m_StorageFile.string());
        return false;
    }
    
    Log::Info("Collections saved to: " + m_StorageFile.string());
    return true;
}

bool CollectionManager::LoadFromFile() {
    if (m_StorageFile.empty()) {
        Log::Warn("Storage file path not set for CollectionManager");
        return false;
    }
    
    if (!FileSystem::Exists(m_StorageFile.string())) {
        Log::Info("Collections file not found: " + m_StorageFile.string());
        return true; // Not an error
    }
    
    std::string jsonString = FileSystem::ReadFile(m_StorageFile.string());
    if (jsonString.empty()) {
        Log::Error("Failed to read collections from: " + m_StorageFile.string());
        return false;
    }
    
    Clear();
    
    // Find collections array
    size_t collectionsPos = jsonString.find("\"collections\"");
    if (collectionsPos == std::string::npos) {
        Log::Warn("No collections array found in collections file");
        return true;
    }
    
    size_t arrayStart = jsonString.find('[', collectionsPos);
    if (arrayStart == std::string::npos) {
        return false;
    }
    
    // Parse each collection object
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
                std::string collectionJson = jsonString.substr(objStart, pos - objStart + 1);
                AssetCollection collection = AssetCollection::FromJson(collectionJson);
                if (!collection.name.empty()) {
                    m_Collections.push_back(collection);
                }
            }
        }
        pos++;
    }
    
    Log::Info("Loaded " + std::to_string(m_Collections.size()) + " collections from: " + m_StorageFile.string());
    return true;
}

} // namespace LGE

