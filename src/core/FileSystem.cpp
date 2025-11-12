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

#include "LGE/core/FileSystem.h"
#include "LGE/core/Log.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace LGE {

std::string FileSystem::ReadFile(const std::string& filepath) {
    // Try multiple paths: relative to current directory, relative to executable, and absolute
    std::vector<std::string> pathsToTry;
    
    // First try the path as-is (relative to current working directory)
    pathsToTry.push_back(filepath);
    
    // Try relative to executable directory (for when running from build/bin/Release/)
    #ifdef _WIN32
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
    // Go up from build/bin/Release/ to project root
    std::filesystem::path projectRoot = exeDir.parent_path().parent_path().parent_path();
    pathsToTry.push_back((projectRoot / filepath).string());
    #else
    // For Linux/Mac, try relative to executable
    char exePath[1024];
    ssize_t count = readlink("/proc/self/exe", exePath, 1024);
    if (count != -1) {
        std::filesystem::path exeDir = std::filesystem::path(std::string(exePath, count)).parent_path();
        std::filesystem::path projectRoot = exeDir.parent_path().parent_path().parent_path();
        pathsToTry.push_back((projectRoot / filepath).string());
    }
    #endif
    
    for (const auto& path : pathsToTry) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
    }
    
    Log::Error("Failed to open file: " + filepath);
    return "";
}

} // namespace LGE

