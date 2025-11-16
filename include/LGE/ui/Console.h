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

#include "LGE/core/Log.h"
#include <string>
#include <vector>
#include <mutex>

// Forward declaration
struct ImVec4;

namespace LGE {

struct LogEntry {
    LogLevel level;
    std::string message;
    std::string timestamp;
    
    LogEntry(LogLevel lvl, const std::string& msg, const std::string& time)
        : level(lvl), message(msg), timestamp(time) {}
};

class Console {
public:
    Console();
    ~Console();

    void OnUIRender();
    
    // Add a log entry
    void AddLog(LogLevel level, const std::string& message);
    
    // Clear all logs
    void Clear();
    
    // Filter settings
    void SetShowTrace(bool show) { m_ShowTrace = show; }
    void SetShowInfo(bool show) { m_ShowInfo = show; }
    void SetShowWarn(bool show) { m_ShowWarn = show; }
    void SetShowError(bool show) { m_ShowError = show; }
    void SetShowFatal(bool show) { m_ShowFatal = show; }
    
    bool GetShowTrace() const { return m_ShowTrace; }
    bool GetShowInfo() const { return m_ShowInfo; }
    bool GetShowWarn() const { return m_ShowWarn; }
    bool GetShowError() const { return m_ShowError; }
    bool GetShowFatal() const { return m_ShowFatal; }
    
    // Auto-scroll to bottom
    void SetAutoScroll(bool autoScroll) { m_AutoScroll = autoScroll; }
    bool GetAutoScroll() const { return m_AutoScroll; }

private:
    void RenderToolbar();
    void RenderLogs();
    struct ImVec4 GetLogColor(LogLevel level) const;
    std::string GetLogLevelString(LogLevel level) const;
    
    std::vector<LogEntry> m_Logs;
    std::mutex m_LogsMutex;
    int m_MaxLogs;  // Maximum number of logs to keep
    
    // Filter flags
    bool m_ShowTrace;
    bool m_ShowInfo;
    bool m_ShowWarn;
    bool m_ShowError;
    bool m_ShowFatal;
    
    // UI state
    bool m_AutoScroll;
    char m_SearchBuffer[256];
};

} // namespace LGE

