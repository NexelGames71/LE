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

#include "LGE/core/assets/DependencyGraph.h"
#include "LGE/core/assets/AssetRegistry.h"
#include "LGE/core/Log.h"
#include "LGE/core/filesystem/FileSystem.h"
#include <algorithm>
#include <queue>
#include <sstream>

namespace LGE {

DependencyGraph::DependencyGraph(AssetRegistry* reg)
    : m_Registry(reg)
{
}

void DependencyGraph::AddDependency(const GUID& asset, const GUID& dependency) {
    if (asset == dependency) {
        Log::Warn("Asset cannot depend on itself: " + asset.ToString());
        return;
    }
    
    // Add to dependencies map
    m_Dependencies[asset].insert(dependency);
    
    // Add to reverse dependents map
    m_Dependents[dependency].insert(asset);
    
    Log::Info("Added dependency: " + asset.ToString() + " -> " + dependency.ToString());
}

void DependencyGraph::RemoveDependencies(const GUID& asset) {
    // Remove from dependencies map
    auto depIt = m_Dependencies.find(asset);
    if (depIt != m_Dependencies.end()) {
        // Remove from reverse dependents map
        for (const auto& dep : depIt->second) {
            auto depIt2 = m_Dependents.find(dep);
            if (depIt2 != m_Dependents.end()) {
                depIt2->second.erase(asset);
                if (depIt2->second.empty()) {
                    m_Dependents.erase(depIt2);
                }
            }
        }
        m_Dependencies.erase(depIt);
    }
    
    // Also remove from dependents map (assets that depend on this one)
    auto depIt3 = m_Dependents.find(asset);
    if (depIt3 != m_Dependents.end()) {
        // Remove from dependencies map of dependents
        for (const auto& dependent : depIt3->second) {
            auto depIt4 = m_Dependencies.find(dependent);
            if (depIt4 != m_Dependencies.end()) {
                depIt4->second.erase(asset);
                if (depIt4->second.empty()) {
                    m_Dependencies.erase(depIt4);
                }
            }
        }
        m_Dependents.erase(depIt3);
    }
}

void DependencyGraph::RemoveDependency(const GUID& asset, const GUID& dependency) {
    // Remove from dependencies map
    auto depIt = m_Dependencies.find(asset);
    if (depIt != m_Dependencies.end()) {
        depIt->second.erase(dependency);
        if (depIt->second.empty()) {
            m_Dependencies.erase(depIt);
        }
    }
    
    // Remove from reverse dependents map
    auto depIt2 = m_Dependents.find(dependency);
    if (depIt2 != m_Dependents.end()) {
        depIt2->second.erase(asset);
        if (depIt2->second.empty()) {
            m_Dependents.erase(depIt2);
        }
    }
}

std::vector<GUID> DependencyGraph::GetDependencies(const GUID& asset) const {
    std::vector<GUID> result;
    auto it = m_Dependencies.find(asset);
    if (it != m_Dependencies.end()) {
        result.assign(it->second.begin(), it->second.end());
    }
    return result;
}

void DependencyGraph::CollectDependenciesRecursive(const GUID& asset, std::unordered_set<GUID>& visited, std::vector<GUID>& result) const {
    if (visited.count(asset)) {
        return; // Already visited, prevent infinite loops
    }
    visited.insert(asset);
    
    auto it = m_Dependencies.find(asset);
    if (it != m_Dependencies.end()) {
        for (const auto& dep : it->second) {
            if (std::find(result.begin(), result.end(), dep) == result.end()) {
                result.push_back(dep);
            }
            CollectDependenciesRecursive(dep, visited, result);
        }
    }
}

std::vector<GUID> DependencyGraph::GetAllDependencies(const GUID& asset, bool recursive) const {
    if (!recursive) {
        return GetDependencies(asset);
    }
    
    std::vector<GUID> result;
    std::unordered_set<GUID> visited;
    CollectDependenciesRecursive(asset, visited, result);
    return result;
}

std::vector<GUID> DependencyGraph::GetDependents(const GUID& asset) const {
    std::vector<GUID> result;
    auto it = m_Dependents.find(asset);
    if (it != m_Dependents.end()) {
        result.assign(it->second.begin(), it->second.end());
    }
    return result;
}

void DependencyGraph::CollectDependentsRecursive(const GUID& asset, std::unordered_set<GUID>& visited, std::vector<GUID>& result) const {
    if (visited.count(asset)) {
        return; // Already visited
    }
    visited.insert(asset);
    
    auto it = m_Dependents.find(asset);
    if (it != m_Dependents.end()) {
        for (const auto& dependent : it->second) {
            if (std::find(result.begin(), result.end(), dependent) == result.end()) {
                result.push_back(dependent);
            }
            CollectDependentsRecursive(dependent, visited, result);
        }
    }
}

std::vector<GUID> DependencyGraph::GetAllDependents(const GUID& asset, bool recursive) const {
    if (!recursive) {
        return GetDependents(asset);
    }
    
    std::vector<GUID> result;
    std::unordered_set<GUID> visited;
    CollectDependentsRecursive(asset, visited, result);
    return result;
}

bool DependencyGraph::HasCycleDFS(const GUID& asset, std::unordered_set<GUID>& visited, std::unordered_set<GUID>& recStack) const {
    visited.insert(asset);
    recStack.insert(asset);
    
    auto it = m_Dependencies.find(asset);
    if (it != m_Dependencies.end()) {
        for (const auto& dep : it->second) {
            if (!visited.count(dep)) {
                if (HasCycleDFS(dep, visited, recStack)) {
                    return true;
                }
            } else if (recStack.count(dep)) {
                return true; // Found a cycle
            }
        }
    }
    
    recStack.erase(asset);
    return false;
}

bool DependencyGraph::HasCircularDependency(const GUID& asset) const {
    std::unordered_set<GUID> visited;
    std::unordered_set<GUID> recStack;
    return HasCycleDFS(asset, visited, recStack);
}

std::vector<GUID> DependencyGraph::GetLoadOrder(const std::vector<GUID>& assets) const {
    // Topological sort using Kahn's algorithm
    std::vector<GUID> result;
    std::unordered_map<GUID, int> inDegree;
    std::queue<GUID> queue;
    
    // Calculate in-degrees
    for (const auto& asset : assets) {
        inDegree[asset] = 0;
    }
    
    for (const auto& asset : assets) {
        auto deps = GetDependencies(asset);
        for (const auto& dep : deps) {
            if (std::find(assets.begin(), assets.end(), dep) != assets.end()) {
                inDegree[asset]++;
            }
        }
    }
    
    // Find all nodes with in-degree 0
    for (const auto& asset : assets) {
        if (inDegree[asset] == 0) {
            queue.push(asset);
        }
    }
    
    // Process nodes
    while (!queue.empty()) {
        GUID current = queue.front();
        queue.pop();
        result.push_back(current);
        
        // Find dependents of current node
        auto dependents = GetDependents(current);
        for (const auto& dependent : dependents) {
            if (std::find(assets.begin(), assets.end(), dependent) != assets.end()) {
                inDegree[dependent]--;
                if (inDegree[dependent] == 0) {
                    queue.push(dependent);
                }
            }
        }
    }
    
    // If result size != assets size, there's a cycle
    if (result.size() != assets.size()) {
        Log::Warn("Circular dependency detected in load order calculation");
        // Return assets in original order as fallback
        return assets;
    }
    
    return result;
}

bool DependencyGraph::Validate() const {
    // Check that reverse mappings are consistent
    for (const auto& pair : m_Dependencies) {
        const GUID& asset = pair.first;
        for (const auto& dep : pair.second) {
            auto depIt = m_Dependents.find(dep);
            if (depIt == m_Dependents.end() || depIt->second.count(asset) == 0) {
                Log::Error("Inconsistent dependency graph: " + asset.ToString() + " -> " + dep.ToString());
                return false;
            }
        }
    }
    
    // Check reverse mapping consistency
    for (const auto& pair : m_Dependents) {
        const GUID& asset = pair.first;
        for (const auto& dependent : pair.second) {
            auto depIt = m_Dependencies.find(dependent);
            if (depIt == m_Dependencies.end() || depIt->second.count(asset) == 0) {
                Log::Error("Inconsistent dependency graph: " + dependent.ToString() + " depends on " + asset.ToString());
                return false;
            }
        }
    }
    
    return true;
}

void DependencyGraph::Clear() {
    m_Dependencies.clear();
    m_Dependents.clear();
    Log::Info("Dependency graph cleared");
}

bool DependencyGraph::SaveToFile(const std::filesystem::path& path) const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"dependencies\": {\n";
    
    size_t assetIndex = 0;
    for (const auto& pair : m_Dependencies) {
        json << "    \"" << pair.first.ToString() << "\": [\n";
        size_t depIndex = 0;
        for (const auto& dep : pair.second) {
            json << "      \"" << dep.ToString() << "\"";
            if (depIndex < pair.second.size() - 1) {
                json << ",";
            }
            json << "\n";
            depIndex++;
        }
        json << "    ]";
        if (assetIndex < m_Dependencies.size() - 1) {
            json << ",";
        }
        json << "\n";
        assetIndex++;
    }
    
    json << "  }\n";
    json << "}\n";
    
    if (!FileSystem::WriteFile(path.string(), json.str())) {
        Log::Error("Failed to save dependency graph to: " + path.string());
        return false;
    }
    
    Log::Info("Dependency graph saved to: " + path.string());
    return true;
}

bool DependencyGraph::LoadFromFile(const std::filesystem::path& path) {
    if (!FileSystem::Exists(path.string())) {
        Log::Info("Dependency graph file not found: " + path.string());
        return true; // Not an error
    }
    
    std::string jsonString = FileSystem::ReadFile(path.string());
    if (jsonString.empty()) {
        Log::Error("Failed to read dependency graph from: " + path.string());
        return false;
    }
    
    Clear();
    
    // Find dependencies object
    size_t depsPos = jsonString.find("\"dependencies\"");
    if (depsPos == std::string::npos) {
        Log::Warn("No dependencies object found in dependency graph file");
        return true; // Not an error, just empty
    }
    
    size_t objStart = jsonString.find('{', depsPos);
    if (objStart == std::string::npos) {
        Log::Error("Invalid dependency graph JSON format");
        return false;
    }
    
    // Parse each asset entry
    size_t pos = objStart + 1;
    while (pos < jsonString.length()) {
        // Find next GUID key
        size_t guidStart = jsonString.find('"', pos);
        if (guidStart == std::string::npos) {
            break;
        }
        
        size_t guidEnd = jsonString.find('"', guidStart + 1);
        if (guidEnd == std::string::npos) {
            break;
        }
        
        std::string guidStr = jsonString.substr(guidStart + 1, guidEnd - guidStart - 1);
        GUID asset = GUID::FromString(guidStr);
        if (!asset.IsValid()) {
            Log::Warn("Invalid GUID in dependency graph: " + guidStr);
            pos = guidEnd + 1;
            continue;
        }
        
        // Find array start
        size_t arrayStart = jsonString.find('[', guidEnd);
        if (arrayStart == std::string::npos) {
            break;
        }
        
        size_t arrayEnd = jsonString.find(']', arrayStart);
        if (arrayEnd == std::string::npos) {
            break;
        }
        
        // Parse dependencies in array
        std::string arrayContent = jsonString.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
        size_t arrayPos = 0;
        while ((arrayPos = arrayContent.find('"', arrayPos)) != std::string::npos) {
            size_t depStart = arrayPos;
            size_t depEnd = arrayContent.find('"', depStart + 1);
            if (depEnd == std::string::npos) {
                break;
            }
            
            std::string depStr = arrayContent.substr(depStart + 1, depEnd - depStart - 1);
            GUID dep = GUID::FromString(depStr);
            if (dep.IsValid()) {
                AddDependency(asset, dep);
            }
            
            arrayPos = depEnd + 1;
        }
        
        pos = arrayEnd + 1;
    }
    
    Log::Info("Dependency graph loaded from: " + path.string());
    return true;
}

} // namespace LGE

