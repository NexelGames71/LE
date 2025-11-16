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

#ifdef __APPLE__

#include "LGE/core/filesystem/FileSystemWatcher.h"
#include <CoreServices/CoreServices.h>
#include <thread>
#include <atomic>

namespace LGE {

class macOSFileSystemWatcher : public FileSystemWatcher {
private:
    FSEventStreamRef m_EventStream;
    CFRunLoopRef m_RunLoop;
    std::thread m_WatchThread;
    std::atomic<bool> m_ShouldStop;
    
    void WatchThreadFunc();
    static void FSEventCallback(
        ConstFSEventStreamRef streamRef,
        void* clientCallBackInfo,
        size_t numEvents,
        void* eventPaths,
        const FSEventStreamEventFlags eventFlags[],
        const FSEventStreamEventId eventIds[]
    );

public:
    macOSFileSystemWatcher();
    ~macOSFileSystemWatcher();
    
    bool StartWatching(const std::filesystem::path& directory) override;
    void StopWatching() override;
};

} // namespace LGE

#endif // __APPLE__



