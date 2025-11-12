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

namespace LGE {

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

