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

#include "LGE/ui/ValidationReportWindow.h"
#include "LGE/core/assets/AssetValidator.h"
#include "imgui.h"
#include <algorithm>
#include <cstring>

namespace LGE {

ValidationReportWindow::ValidationReportWindow(AssetValidator* validator)
    : m_Validator(validator)
    , m_IsOpen(false)
    , m_ShowInfo(true)
    , m_ShowWarnings(true)
    , m_ShowErrors(true)
{
    m_WindowTitle = "Asset Validation Report";
}

void ValidationReportWindow::Show(const std::vector<AssetValidator::ValidationResult>& results) {
    m_Results = results;
    m_IsOpen = true;
}

std::vector<AssetValidator::ValidationResult> ValidationReportWindow::GetFilteredResults() const {
    std::vector<AssetValidator::ValidationResult> filtered;
    
    for (const auto& result : m_Results) {
        // Severity filter
        bool show = false;
        switch (result.severity) {
            case AssetValidator::ValidationSeverity::Info:
                show = m_ShowInfo;
                break;
            case AssetValidator::ValidationSeverity::Warning:
                show = m_ShowWarnings;
                break;
            case AssetValidator::ValidationSeverity::Error:
                show = m_ShowErrors;
                break;
        }
        
        if (!show) continue;
        
        // Text filter
        if (!m_FilterText.empty()) {
            std::string lowerMessage = result.message;
            std::transform(lowerMessage.begin(), lowerMessage.end(), lowerMessage.begin(), ::tolower);
            std::string lowerFilter = m_FilterText;
            std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
            
            if (lowerMessage.find(lowerFilter) == std::string::npos) {
                continue;
            }
        }
        
        filtered.push_back(result);
    }
    
    return filtered;
}

void ValidationReportWindow::Render() {
    if (!m_IsOpen) {
        return;
    }
    
    if (!m_Validator) {
        m_IsOpen = false;
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin(m_WindowTitle.c_str(), &m_IsOpen)) {
        // Summary
        AssetValidator::ValidationSummary summary = m_Validator->GetSummary(m_Results);
        ImGui::Text("Summary: %d Info, %d Warnings, %d Errors", 
            summary.infoCount, summary.warningCount, summary.errorCount);
        if (summary.autoFixableCount > 0) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "(%d auto-fixable)", summary.autoFixableCount);
        }
        ImGui::Separator();
        
        // Filters
        ImGui::Checkbox("Info", &m_ShowInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warnings", &m_ShowWarnings);
        ImGui::SameLine();
        ImGui::Checkbox("Errors", &m_ShowErrors);
        ImGui::SameLine();
        ImGui::Text("Filter:");
        ImGui::SameLine();
        char filterBuf[256] = {0};
        strncpy(filterBuf, m_FilterText.c_str(), sizeof(filterBuf) - 1);
        if (ImGui::InputText("##Filter", filterBuf, sizeof(filterBuf))) {
            m_FilterText = filterBuf;
        }
        
        ImGui::Separator();
        
        // Results table
        std::vector<AssetValidator::ValidationResult> filtered = GetFilteredResults();
        
        if (filtered.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No results to display");
        } else {
            if (ImGui::BeginTable("ValidationResults", 4, 
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
                
                ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_WidthFixed, 80);
                ImGui::TableSetupColumn("Asset", ImGuiTableColumnFlags_WidthFixed, 200);
                ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 100);
                ImGui::TableHeadersRow();
                
                for (const auto& result : filtered) {
                    ImGui::TableNextRow();
                    
                    // Severity
                    ImGui::TableSetColumnIndex(0);
                    ImVec4 severityColor;
                    const char* severityText;
                    switch (result.severity) {
                        case AssetValidator::ValidationSeverity::Info:
                            severityColor = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);
                            severityText = "Info";
                            break;
                        case AssetValidator::ValidationSeverity::Warning:
                            severityColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                            severityText = "Warning";
                            break;
                        case AssetValidator::ValidationSeverity::Error:
                            severityColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                            severityText = "Error";
                            break;
                    }
                    ImGui::TextColored(severityColor, "%s", severityText);
                    
                    // Asset
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", result.asset.ToString().c_str());
                    
                    // Message
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", result.message.c_str());
                    if (!result.details.empty() && ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("%s", result.details.c_str());
                    }
                    
                    // Actions
                    ImGui::TableSetColumnIndex(3);
                    if (result.canAutoFix) {
                        if (ImGui::SmallButton(("Fix##" + result.asset.ToString()).c_str())) {
                            if (result.autoFixFunc) {
                                result.autoFixFunc();
                            }
                        }
                    }
                }
                
                ImGui::EndTable();
            }
        }
        
        ImGui::Separator();
        
        // Auto-fix all button
        int autoFixableCount = summary.autoFixableCount;
        if (autoFixableCount > 0) {
            if (ImGui::Button("Auto-Fix All")) {
                m_Validator->AutoFixIssues(m_Results);
            }
        }
    }
    
    ImGui::End();
    
    if (!m_IsOpen) {
        m_Results.clear();
    }
}

} // namespace LGE

