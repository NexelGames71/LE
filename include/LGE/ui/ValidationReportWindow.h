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

#include "LGE/core/assets/AssetValidator.h"
#include <vector>
#include <string>

namespace LGE {

class ValidationReportWindow {
private:
    std::vector<AssetValidator::ValidationResult> m_Results;
    AssetValidator* m_Validator;
    bool m_IsOpen;
    std::string m_WindowTitle;
    
    // Filtering
    bool m_ShowInfo;
    bool m_ShowWarnings;
    bool m_ShowErrors;
    std::string m_FilterText;

public:
    ValidationReportWindow(AssetValidator* validator);
    
    // Show the window with validation results
    void Show(const std::vector<AssetValidator::ValidationResult>& results);
    
    // Close the window
    void Close() { m_IsOpen = false; }
    
    // Check if window is open
    bool IsOpen() const { return m_IsOpen; }
    
    // Render the window (ImGui)
    void Render();
    
    // Get result count
    size_t GetResultCount() const { return m_Results.size(); }
    
    // Get filtered results
    std::vector<AssetValidator::ValidationResult> GetFilteredResults() const;
};

} // namespace LGE

