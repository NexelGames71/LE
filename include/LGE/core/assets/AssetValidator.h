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
#include <vector>
#include <string>
#include <functional>

namespace LGE {

// Forward declarations
class AssetRegistry;
class DependencyGraph;

class AssetValidator {
public:
    enum class ValidationSeverity {
        Info,
        Warning,
        Error
    };
    
    struct ValidationResult {
        ValidationSeverity severity;
        GUID asset;
        std::string message;
        std::string details;
        bool canAutoFix;
        std::function<bool()> autoFixFunc;
        
        ValidationResult()
            : severity(ValidationSeverity::Info)
            , asset(GUID::Invalid())
            , canAutoFix(false)
        {}
    };

private:
    AssetRegistry* m_Registry;
    DependencyGraph* m_Graph;

public:
    AssetValidator(AssetRegistry* registry, DependencyGraph* graph);
    
    // Validate single asset
    std::vector<ValidationResult> ValidateAsset(const GUID& guid);
    
    // Validate entire project
    std::vector<ValidationResult> ValidateProject(
        std::function<void(int, int)> progressCallback = nullptr
    );
    
    // Common validation checks
    std::vector<ValidationResult> CheckMissingDependencies();
    std::vector<ValidationResult> CheckCircularDependencies();
    std::vector<ValidationResult> CheckDuplicateNames();
    std::vector<ValidationResult> CheckInvalidMetadata();
    std::vector<ValidationResult> CheckOrphanedMetaFiles();
    
    // Auto-fix issues
    bool AutoFixIssues(const std::vector<ValidationResult>& results);
    
    // Get validation summary
    struct ValidationSummary {
        int infoCount;
        int warningCount;
        int errorCount;
        int autoFixableCount;
    };
    ValidationSummary GetSummary(const std::vector<ValidationResult>& results) const;
};

} // namespace LGE

