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

#ifdef __APPLE__

#include "LGE/core/filesystem/FileSystemWatcher_macOS.h"
#include "LGE/core/Log.h"
#include <CoreFoundation/CoreFoundation.h>

namespace LGE {

macOSFileSystemWatcher::macOSFileSystemWatcher()
    : m_EventStream(nullptr)
    , m_RunLoop(nullptr)
    , m_ShouldStop(false)
{
}

macOSFileSystemWatcher::~macOSFileSystemWatcher() {
    StopWatching();
}

bool macOSFileSystemWatcher::StartWatching(const std::filesystem::path& directory) {
    if (m_IsWatching.load()) {
        StopWatching();
    }
    
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        Log::Error("Directory does not exist or is not a directory: " + directory.string());
        return false;
    }
    
    m_WatchedDirectory = directory;
    
    // Convert path to CFString
    std::string dirStr = directory.string();
    CFStringRef path = CFStringCreateWithCString(NULL, dirStr.c_str(), kCFStringEncodingUTF8);
    
    CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void**)&path, 1, NULL);
    
    // Create FSEvent stream
    FSEventStreamContext context = {0, this, NULL, NULL, NULL};
    m_EventStream = FSEventStreamCreate(
        NULL,
        &FSEventCallback,
        &context,
        pathsToWatch,
        kFSEventStreamEventIdSinceNow,
        0.1, // Latency in seconds
        kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagWatchRoot
    );
    
    CFRelease(pathsToWatch);
    CFRelease(path);
    
    if (!m_EventStream) {
        Log::Error("Failed to create FSEvent stream");
        return false;
    }
    
    m_ShouldStop = false;
    m_IsWatching = true;
    m_WatchThread = std::thread(&macOSFileSystemWatcher::WatchThreadFunc, this);
    
    Log::Info("Started watching directory: " + directory.string());
    return true;
}

void macOSFileSystemWatcher::StopWatching() {
    if (!m_IsWatching.load()) {
        return;
    }
    
    m_ShouldStop = true;
    m_IsWatching = false;
    
    if (m_EventStream) {
        FSEventStreamStop(m_EventStream);
        FSEventStreamInvalidate(m_EventStream);
        FSEventStreamRelease(m_EventStream);
        m_EventStream = nullptr;
    }
    
    if (m_RunLoop) {
        CFRunLoopStop(m_RunLoop);
    }
    
    if (m_WatchThread.joinable()) {
        m_WatchThread.join();
    }
    
    Log::Info("Stopped watching directory");
}

void macOSFileSystemWatcher::WatchThreadFunc() {
    m_RunLoop = CFRunLoopGetCurrent();
    FSEventStreamScheduleWithRunLoop(m_EventStream, m_RunLoop, kCFRunLoopDefaultMode);
    FSEventStreamStart(m_EventStream);
    
    CFRunLoopRun();
}

void macOSFileSystemWatcher::FSEventCallback(
    ConstFSEventStreamRef streamRef,
    void* clientCallBackInfo,
    size_t numEvents,
    void* eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[]
) {
    macOSFileSystemWatcher* watcher = static_cast<macOSFileSystemWatcher*>(clientCallBackInfo);
    char** paths = static_cast<char**>(eventPaths);
    
    for (size_t i = 0; i < numEvents; ++i) {
        std::filesystem::path path(paths[i]);
        
        FileChangeEvent event;
        event.path = path;
        event.timestamp = std::time(nullptr);
        
        if (eventFlags[i] & kFSEventStreamEventFlagItemCreated) {
            event.type = FileChangeType::Added;
        } else if (eventFlags[i] & kFSEventStreamEventFlagItemRemoved) {
            event.type = FileChangeType::Removed;
        } else if (eventFlags[i] & kFSEventStreamEventFlagItemModified ||
                   eventFlags[i] & kFSEventStreamEventFlagItemInodeMetaMod) {
            event.type = FileChangeType::Modified;
        } else if (eventFlags[i] & kFSEventStreamEventFlagItemRenamed) {
            event.type = FileChangeType::Renamed;
        }
        
        if (watcher->m_Callback) {
            watcher->m_Callback(event);
        }
    }
}

} // namespace LGE

#endif // __APPLE__



