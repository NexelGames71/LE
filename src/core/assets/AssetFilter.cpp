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

#include "LGE/core/assets/AssetFilter.h"
#include "LGE/core/assets/AssetRegistry.h"
#include <algorithm>
#include <regex>

namespace LGE {

std::vector<GUID> AssetFilter::ApplyFilter(
    const std::vector<GUID>& assets,
    const FilterCriteria& criteria,
    AssetRegistry* registry)
{
    if (!registry) {
        return {};
    }
    
    std::vector<GUID> results;
    
    for (const auto& guid : assets) {
        const AssetMetadata* metadata = registry->GetAsset(guid);
        if (!metadata) {
            continue;
        }
        
        if (Matches(*metadata, criteria)) {
            results.push_back(guid);
        }
    }
    
    return results;
}

bool AssetFilter::Matches(const AssetMetadata& metadata, const FilterCriteria& criteria) {
    // Type filter
    if (!criteria.types.empty()) {
        if (std::find(criteria.types.begin(), criteria.types.end(), metadata.type) == criteria.types.end()) {
            return false;
        }
    }
    
    // Tag filter
    if (!criteria.tags.empty()) {
        bool hasAllTags = true;
        for (const auto& requiredTag : criteria.tags) {
            if (std::find(metadata.tags.begin(), metadata.tags.end(), requiredTag) == metadata.tags.end()) {
                hasAllTags = false;
                break;
            }
        }
        if (!hasAllTags) {
            return false;
        }
    }
    
    // Name pattern filter
    if (!criteria.namePattern.empty()) {
        try {
            std::regex pattern(criteria.namePattern, std::regex_constants::icase);
            if (!std::regex_search(metadata.name, pattern)) {
                return false;
            }
        } catch (const std::regex_error&) {
            // Invalid regex, try simple substring match
            std::string lowerName = metadata.name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            std::string lowerPattern = criteria.namePattern;
            std::transform(lowerPattern.begin(), lowerPattern.end(), lowerPattern.begin(), ::tolower);
            if (lowerName.find(lowerPattern) == std::string::npos) {
                return false;
            }
        }
    }
    
    // Size filters
    if (criteria.minSize.has_value() && metadata.fileSize < criteria.minSize.value()) {
        return false;
    }
    if (criteria.maxSize.has_value() && metadata.fileSize > criteria.maxSize.value()) {
        return false;
    }
    
    // Date filters
    if (criteria.modifiedAfter.has_value() && metadata.lastModified < criteria.modifiedAfter.value()) {
        return false;
    }
    if (criteria.modifiedBefore.has_value() && metadata.lastModified > criteria.modifiedBefore.value()) {
        return false;
    }
    
    // Custom filters
    for (const auto& customFilter : criteria.customFilters) {
        if (!customFilter(metadata)) {
            return false;
        }
    }
    
    return true;
}

} // namespace LGE

