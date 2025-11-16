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

#include <iostream>
#include <string>
#include <functional>
#include <vector>

namespace LGE {

enum class LogLevel {
    Trace = 0,
    Info,
    Warn,
    Error,
    Fatal
};

class Log {
public:
    // Log callbacks (for UI integration)
    using LogCallback = std::function<void(LogLevel level, const std::string& message)>;
    
    static void Trace(const std::string& message);
    static void Info(const std::string& message);
    static void Warn(const std::string& message);
    static void Error(const std::string& message);
    static void Fatal(const std::string& message);
    
    // Register a callback to receive log messages
    static void RegisterCallback(LogCallback callback);
    
    // Unregister a callback
    static void UnregisterCallback(LogCallback callback);

private:
    static void Print(LogLevel level, const std::string& message);
    static std::vector<LogCallback> s_Callbacks;
};

} // namespace LGE

