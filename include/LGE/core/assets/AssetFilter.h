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
#include <string>
#include <optional>
#include <functional>
#include <ctime>

namespace LGE {

// Forward declaration
class AssetRegistry;

class AssetFilter {
public:
    struct FilterCriteria {
        std::vector<AssetType> types;
        std::vector<std::string> tags;
        std::string namePattern;
        
        // Size filters
        std::optional<uint64_t> minSize;
        std::optional<uint64_t> maxSize;
        
        // Date filters
        std::optional<std::time_t> modifiedAfter;
        std::optional<std::time_t> modifiedBefore;
        
        // Custom predicates
        std::vector<std::function<bool(const AssetMetadata&)>> customFilters;
        
        FilterCriteria() {}
    };
    
    static std::vector<GUID> ApplyFilter(
        const std::vector<GUID>& assets,
        const FilterCriteria& criteria,
        AssetRegistry* registry
    );
    
    // Helper to check if metadata matches criteria
    static bool Matches(const AssetMetadata& metadata, const FilterCriteria& criteria);
};

} // namespace LGE

