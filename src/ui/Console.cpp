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

#include "LGE/ui/Console.h"
#include "LGE/core/Log.h"
#include "imgui.h"
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <sstream>

namespace LGE {

Console::Console()
    : m_MaxLogs(1000)
    , m_ShowTrace(true)
    , m_ShowInfo(true)
    , m_ShowWarn(true)
    , m_ShowError(true)
    , m_ShowFatal(true)
    , m_AutoScroll(true)
{
    m_SearchBuffer[0] = '\0';
}

Console::~Console() {
}

void Console::OnUIRender() {
    ImGui::Begin("Console", nullptr);
    
    RenderToolbar();
    ImGui::Separator();
    RenderLogs();
    
    ImGui::End();
}

void Console::RenderToolbar() {
    // Clear button
    if (ImGui::Button("Clear")) {
        Clear();
    }
    ImGui::SameLine();
    
    // Auto-scroll checkbox
    ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
    ImGui::SameLine();
    
    // Filter checkboxes
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::Text("Filters:");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::Checkbox("Trace", &m_ShowTrace);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
    ImGui::Checkbox("Info", &m_ShowInfo);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
    ImGui::Checkbox("Warn", &m_ShowWarn);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
    ImGui::Checkbox("Error", &m_ShowError);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    ImGui::Checkbox("Fatal", &m_ShowFatal);
    ImGui::PopStyleColor();
    
    // Search box
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputText("##Search", m_SearchBuffer, sizeof(m_SearchBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Search logs");
    }
}

void Console::RenderLogs() {
    // Get filtered logs
    std::vector<LogEntry> filteredLogs;
    {
        std::lock_guard<std::mutex> lock(m_LogsMutex);
        
        for (const auto& log : m_Logs) {
            // Check level filter
            bool show = false;
            switch (log.level) {
                case LogLevel::Trace: show = m_ShowTrace; break;
                case LogLevel::Info:  show = m_ShowInfo; break;
                case LogLevel::Warn:  show = m_ShowWarn; break;
                case LogLevel::Error: show = m_ShowError; break;
                case LogLevel::Fatal: show = m_ShowFatal; break;
            }
            
            if (!show) continue;
            
            // Check search filter
            if (m_SearchBuffer[0] != '\0') {
                std::string searchLower = m_SearchBuffer;
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                
                std::string logLower = log.message;
                std::transform(logLower.begin(), logLower.end(), logLower.begin(), ::tolower);
                
                if (logLower.find(searchLower) == std::string::npos) {
                    continue;
                }
            }
            
            filteredLogs.push_back(log);
        }
    }
    
    // Render logs in a child window with scrolling
    ImGui::BeginChild("Logs", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    // Render each log entry with color coding
    for (size_t i = 0; i < filteredLogs.size(); ++i) {
        const auto& log = filteredLogs[i];
        
        // Get color for this log level
        ImVec4 color = GetLogColor(log.level);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        
        // Format: [timestamp] [LEVEL] message
        std::string formatted = "[" + log.timestamp + "] " + GetLogLevelString(log.level) + " " + log.message;
        
        // Use Selectable for each log entry to allow selection and copying
        // This allows clicking to select and Ctrl+C to copy
        ImGui::PushID(static_cast<int>(i));
        ImGui::Selectable(formatted.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);
        
        // Context menu for easy copying
        if (ImGui::BeginPopupContextItem("LogContextMenu")) {
            if (ImGui::MenuItem("Copy")) {
                ImGui::SetClipboardText(formatted.c_str());
            }
            if (ImGui::MenuItem("Copy All")) {
                // Copy all filtered logs
                std::string allLogs;
                for (const auto& l : filteredLogs) {
                    allLogs += "[" + l.timestamp + "] " + GetLogLevelString(l.level) + " " + l.message + "\n";
                }
                ImGui::SetClipboardText(allLogs.c_str());
            }
            ImGui::EndPopup();
        }
        
        ImGui::PopID();
        ImGui::PopStyleColor();
    }
    
    // Auto-scroll to bottom
    if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

struct ImVec4 Console::GetLogColor(LogLevel level) const {
    switch (level) {
        case LogLevel::Trace: return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);  // Gray
        case LogLevel::Info:  return ImVec4(0.3f, 0.7f, 1.0f, 1.0f);  // Blue
        case LogLevel::Warn:  return ImVec4(1.0f, 0.8f, 0.0f, 1.0f);  // Yellow
        case LogLevel::Error: return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);  // Red
        case LogLevel::Fatal: return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Bright red
        default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // White
    }
}

std::string Console::GetLogLevelString(LogLevel level) const {
    switch (level) {
        case LogLevel::Trace: return "[TRACE]";
        case LogLevel::Info:  return "[INFO] ";
        case LogLevel::Warn:  return "[WARN] ";
        case LogLevel::Error: return "[ERROR]";
        case LogLevel::Fatal: return "[FATAL]";
        default: return "[UNKNOWN]";
    }
}

void Console::AddLog(LogLevel level, const std::string& message) {
    // Get current time
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    char timeStr[32];
    std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &tm);
    
    std::lock_guard<std::mutex> lock(m_LogsMutex);
    
    m_Logs.emplace_back(level, message, std::string(timeStr));
    
    // Limit log count
    if (m_Logs.size() > static_cast<size_t>(m_MaxLogs)) {
        m_Logs.erase(m_Logs.begin());
    }
}

void Console::Clear() {
    std::lock_guard<std::mutex> lock(m_LogsMutex);
    m_Logs.clear();
}

} // namespace LGE

