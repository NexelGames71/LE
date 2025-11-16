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

#ifdef __linux__

#include "LGE/core/filesystem/FileSystemWatcher.h"
#include <sys/inotify.h>
#include <thread>
#include <atomic>
#include <map>

namespace LGE {

class LinuxFileSystemWatcher : public FileSystemWatcher {
private:
    int m_InotifyFd;
    std::thread m_WatchThread;
    std::atomic<bool> m_ShouldStop;
    std::map<int, std::filesystem::path> m_WatchDescriptors;
    
    void WatchThreadFunc();
    void AddWatchRecursive(const std::filesystem::path& path, int wd);

public:
    LinuxFileSystemWatcher();
    ~LinuxFileSystemWatcher();
    
    bool StartWatching(const std::filesystem::path& directory) override;
    void StopWatching() override;
};

} // namespace LGE

#endif // __linux__



