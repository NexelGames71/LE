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

#include "LGE/core/filesystem/FileSystemWatcher.h"

#ifdef _WIN32
#include "LGE/core/filesystem/FileSystemWatcher_Windows.h"
#elif defined(__linux__)
#include "LGE/core/filesystem/FileSystemWatcher_Linux.h"
#elif defined(__APPLE__)
#include "LGE/core/filesystem/FileSystemWatcher_macOS.h"
#endif

namespace LGE {

std::unique_ptr<FileSystemWatcher> FileSystemWatcher::Create() {
#ifdef _WIN32
    return std::make_unique<WindowsFileSystemWatcher>();
#elif defined(__linux__)
    return std::make_unique<LinuxFileSystemWatcher>();
#elif defined(__APPLE__)
    return std::make_unique<macOSFileSystemWatcher>();
#else
    // Fallback implementation (polling-based)
    return nullptr;
#endif
}

} // namespace LGE



