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
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <filesystem>

namespace LGE {

// Forward declaration
class AssetRegistry;

class DependencyGraph {
private:
    // Asset -> Assets it depends on
    std::unordered_map<GUID, std::unordered_set<GUID>> m_Dependencies;
    
    // Asset -> Assets that depend on it (reverse lookup)
    std::unordered_map<GUID, std::unordered_set<GUID>> m_Dependents;
    
    AssetRegistry* m_Registry;

    // Helper for recursive dependency collection
    void CollectDependenciesRecursive(const GUID& asset, std::unordered_set<GUID>& visited, std::vector<GUID>& result) const;
    
    // Helper for recursive dependent collection
    void CollectDependentsRecursive(const GUID& asset, std::unordered_set<GUID>& visited, std::vector<GUID>& result) const;
    
    // Helper for cycle detection (DFS)
    bool HasCycleDFS(const GUID& asset, std::unordered_set<GUID>& visited, std::unordered_set<GUID>& recStack) const;

public:
    DependencyGraph(AssetRegistry* reg);
    
    // Add a dependency relationship
    void AddDependency(const GUID& asset, const GUID& dependency);
    
    // Remove all dependencies for an asset
    void RemoveDependencies(const GUID& asset);
    
    // Remove a specific dependency
    void RemoveDependency(const GUID& asset, const GUID& dependency);
    
    // Get direct dependencies
    std::vector<GUID> GetDependencies(const GUID& asset) const;
    
    // Get all dependencies recursively
    std::vector<GUID> GetAllDependencies(const GUID& asset, bool recursive = true) const;
    
    // Get what depends on this asset (direct dependents)
    std::vector<GUID> GetDependents(const GUID& asset) const;
    
    // Get all dependents recursively
    std::vector<GUID> GetAllDependents(const GUID& asset, bool recursive = true) const;
    
    // Check for circular dependencies
    bool HasCircularDependency(const GUID& asset) const;
    
    // Get load order for assets (topological sort)
    std::vector<GUID> GetLoadOrder(const std::vector<GUID>& assets) const;
    
    // Validate graph integrity
    bool Validate() const;
    
    // Clear all dependencies
    void Clear();
    
    // Get statistics
    size_t GetDependencyCount() const { return m_Dependencies.size(); }
    size_t GetDependentCount() const { return m_Dependents.size(); }
    
    // Persistence
    bool SaveToFile(const std::filesystem::path& path) const;
    bool LoadFromFile(const std::filesystem::path& path);
};

} // namespace LGE

