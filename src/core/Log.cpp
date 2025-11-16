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

#include "LGE/core/Log.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>

namespace LGE {

std::vector<Log::LogCallback> Log::s_Callbacks;

void Log::Print(LogLevel level, const std::string& message) {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::string levelStr;
    switch (level) {
        case LogLevel::Trace: levelStr = "[TRACE]"; break;
        case LogLevel::Info:  levelStr = "[INFO] "; break;
        case LogLevel::Warn:  levelStr = "[WARN] "; break;
        case LogLevel::Error: levelStr = "[ERROR]"; break;
        case LogLevel::Fatal: levelStr = "[FATAL]"; break;
    }

    std::cout << std::put_time(&tm, "%H:%M:%S") << " " << levelStr << " " << message << std::endl;
    
    // Notify all registered callbacks
    for (const auto& callback : s_Callbacks) {
        if (callback) {
            callback(level, message);
        }
    }
}

void Log::RegisterCallback(LogCallback callback) {
    s_Callbacks.push_back(callback);
}

void Log::UnregisterCallback(LogCallback callback) {
    // Note: std::function objects cannot be directly compared with ==
    // This is a simplified implementation that may not work perfectly
    // For production use, consider using callback IDs or a different mechanism
    // For now, we'll use a workaround by comparing target addresses
    // This may not work for all cases, especially with lambdas
    s_Callbacks.erase(
        std::remove_if(s_Callbacks.begin(), s_Callbacks.end(),
            [&callback](const LogCallback& cb) {
                // Try to compare by checking if they're the same object
                // This is a workaround and may not work for all cases
                return cb.target<void(LogLevel, const std::string&)>() == 
                       callback.target<void(LogLevel, const std::string&)>();
            }),
        s_Callbacks.end()
    );
}

void Log::Trace(const std::string& message) {
    Print(LogLevel::Trace, message);
}

void Log::Info(const std::string& message) {
    Print(LogLevel::Info, message);
}

void Log::Warn(const std::string& message) {
    Print(LogLevel::Warn, message);
}

void Log::Error(const std::string& message) {
    Print(LogLevel::Error, message);
}

void Log::Fatal(const std::string& message) {
    Print(LogLevel::Fatal, message);
}

} // namespace LGE

