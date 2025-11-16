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

#ifdef _WIN32

#include "LGE/core/filesystem/FileSystemWatcher.h"
#include <windows.h>
#include <thread>
#include <atomic>

namespace LGE {

class WindowsFileSystemWatcher : public FileSystemWatcher {
private:
    HANDLE m_DirectoryHandle;
    HANDLE m_StopEvent;
    std::thread m_WatchThread;
    
    void WatchThreadFunc();

public:
    WindowsFileSystemWatcher();
    ~WindowsFileSystemWatcher();
    
    bool StartWatching(const std::filesystem::path& directory) override;
    void StopWatching() override;
};

} // namespace LGE

#endif // _WIN32



